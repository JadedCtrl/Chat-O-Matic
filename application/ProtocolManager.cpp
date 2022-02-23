/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include <stdio.h>
#include <image.h>

#include <Bitmap.h>
#include <Directory.h>
#include <Entry.h>
#include <Handler.h>

#include "Account.h"
#include "AppMessages.h"
#include "ChatProtocolMessages.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "ChatProtocol.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"
#include "Utils.h"

static ProtocolManager*	fInstance = NULL;


bool
ProtocolManager::Init(BDirectory dir, BHandler* target)
{
	BEntry entry;
	BPath path;
	bool ret = false;

	dir.Rewind();

	while (dir.GetNextEntry(&entry) == B_OK) {
		path = BPath(&entry);

		// Load protocol addon
		image_id id = load_add_on(path.Path());
		if (id < 0)
			continue;

		// Refuse to load add-on under some circumstances…
		ChatProtocolAddOn* addOn = new ChatProtocolAddOn(id, path.Path());
		if (addOn->Version() != APP_VERSION || ProtocolAddOn(addOn->Signature()) != NULL) {
			if (addOn->Version() != APP_VERSION)
				printf("%s not loaded, due to insufficient version (%i v %i).\n",
					addOn->Signature(), addOn->Version(), APP_VERSION);
			else if (ProtocolAddOn(addOn->Signature()) != NULL)
				printf("%s not loaded, due to another instance already having been loaded.\n",
					addOn->Signature());
			delete addOn;
			continue;
		}
		ret = true;

		// If add-on has multiple protocols, also load them
		for (int32 i = 0; i < addOn->CountProtocols(); i++) {
			ChatProtocolAddOn* subAddOn = addOn;
			if (i > 0)
				subAddOn = new ChatProtocolAddOn(id, path.Path(), i);

			ChatProtocol* proto = subAddOn->Protocol();

			fAddOnMap.AddItem(proto->Signature(), subAddOn);
			_LoadAccounts(path.Path(), subAddOn, i, target);
			delete proto;
		}
	}
	return ret;
}


ProtocolManager::ProtocolManager()
{
}


ProtocolManager*
ProtocolManager::Get()
{
	if (fInstance == NULL)
		fInstance = new ProtocolManager();
	return fInstance;
}


uint32
ProtocolManager::CountProtocolAddOns() const
{
	return fAddOnMap.CountItems();
}


ChatProtocolAddOn*
ProtocolManager::ProtocolAddOnAt(uint32 i) const
{
	return fAddOnMap.ValueAt(i);
}


ChatProtocolAddOn*
ProtocolManager::ProtocolAddOn(const char* signature)
{
	return fAddOnMap.ValueFor(signature);
}


uint32
ProtocolManager::CountProtocolInstances() const
{
	return fProtocolMap.CountItems();
}


ChatProtocol*
ProtocolManager::ProtocolInstanceAt(uint32 i) const
{
	return fProtocolMap.ValueAt(i);
}


ChatProtocol*	
ProtocolManager::ProtocolInstance(bigtime_t identifier)
{
	return fProtocolMap.ValueFor(identifier);
}


void
ProtocolManager::AddAccount(ChatProtocolAddOn* addOn, const char* account,
	BHandler* target)
{
	// If already active, don't double-dip!
	bool active = false;
	_Server()->GetActiveAccounts().ValueFor(BString(account), &active);
	if (active == true)
		return;

	bigtime_t instanceId = system_time();
	ChatProtocol* cayap = addOn->Protocol();
	Account* acc =
		new Account(instanceId, cayap, account, addOn->Signature(), target);

	// If account is disabled, just let it go
	if (acc->InitCheck() == B_DONT_DO_THAT) {
		delete acc;
		return;
	}
	// Send a "whoops" notification if hits a failure
	else if (acc->InitCheck() != B_OK) {
		BMessage error(APP_ACCOUNT_FAILED);
		cayap->Icon()->Archive(&error);
		error.AddString("name", account);
		_MainWin()->MessageReceived(&error);
		return;
	}

	fProtocolMap.AddItem(instanceId, cayap);

	_Server()->AddProtocolLooper(instanceId, cayap);
}


void
ProtocolManager::EnableAccount(ProtocolSettings* settings, const char* account)
{
	BMessage* msg = NULL;
	if (settings->Load(account, &msg) == B_OK) {
		if (msg->HasBool("disabled"))
			msg->ReplaceBool("disabled", false);
		else
			msg->AddBool("disabled", false);
		settings->Save(account, *msg);
	}
	AddAccount(settings->AddOn(), account, _MainWin());
}


void
ProtocolManager::DisableAccount(ProtocolSettings* settings, const char* account)
{
	bool active = false;
	int64 instance
		= _Server()->GetActiveAccounts().ValueFor(BString(account), &active);
	if (active == false)
		return;

	BMessage* msg = NULL;
	if (settings->Load(account, &msg) == B_OK) {
		if (msg->HasBool("disabled"))
			msg->ReplaceBool("disabled", true);
		else
			msg->AddBool("disabled", true);
		settings->Save(account, *msg);
	}

	BMessage remove(IM_MESSAGE);
	remove.AddInt32("im_what", IM_PROTOCOL_DISABLE);
	remove.AddInt64("instance", instance);
	_MainWin()->PostMessage(&remove);
}


void
ProtocolManager::ToggleAccount(ProtocolSettings* settings, const char* account)
{
	bool active = false;
	int64 instance
		= _Server()->GetActiveAccounts().ValueFor(BString(account), &active);

	if (active == true)
		DisableAccount(settings, account);
	else
		EnableAccount(settings, account);
}


void
ProtocolManager::_LoadAccounts(const char* image_path, ChatProtocolAddOn* addOn,
	int protoIndex, BHandler* target)
{
	// Find accounts path for this protocol
	BPath path(AccountPath(addOn->Signature(), addOn->Protocol()->Signature()));
	if (path.InitCheck() != B_OK)
		return;

	BDirectory dir(path.Path());
	BEntry entry;
	bool firstDone = false;

	while (dir.GetNextEntry(&entry) == B_OK)
		_LoadAccount(addOn, entry, target);
}


void
ProtocolManager::_LoadAccount(const char* imagePath, BEntry accountEntry,
	int protoIndex, BHandler* target)
{
	image_id id = load_add_on(imagePath);
	if (id < 0)
		return;

	// If add-on's API version fits then load accounts…
	ChatProtocolAddOn* addOn = new ChatProtocolAddOn(id, imagePath, protoIndex);
	if (addOn->Version() != APP_VERSION)
		return;

	_LoadAccount(addOn, accountEntry, target);
}


void
ProtocolManager::_LoadAccount(ChatProtocolAddOn* addOn, BEntry accountEntry,
	BHandler* target)
{
	BFile file(&accountEntry, B_READ_ONLY);
	BMessage msg;

	if (msg.Unflatten(&file) == B_OK) {
		char buffer[B_PATH_NAME_LENGTH];
		if (accountEntry.GetName(buffer) == B_OK) {
			printf("Found %s for protocol %s!\n", buffer, addOn->Protocol()->Signature());
			AddAccount(addOn, buffer, target);
		}
	}
}


MainWindow*
ProtocolManager::_MainWin()
{
	return ((TheApp*)be_app)->GetMainWindow();
}


Server*
ProtocolManager::_Server()
{
	MainWindow* win = _MainWin();
	return win ? win->GetServer() : NULL;
}
