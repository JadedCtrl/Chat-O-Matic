/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <stdio.h>
#include <image.h>

#include <Directory.h>
#include <Entry.h>
#include <Handler.h>

#include "Account.h"
#include "ProtocolManager.h"
#include "CayaProtocol.h"
#include "CayaUtils.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"

static ProtocolManager*	fInstance = NULL;


void
ProtocolManager::Init(BDirectory dir, BHandler* target)
{
	BEntry entry;
	BPath path;

	dir.Rewind();

	while (dir.GetNextEntry(&entry) == B_OK) {
		path = BPath(&entry);

		// Load protocol addon
		image_id id = load_add_on(path.Path());
		if (id < 0)
			continue;

		// If add-on's API version fits then load accounts...
		CayaProtocolAddOn* addOn = new CayaProtocolAddOn(id, path.Path());
		if (addOn->Version() != CAYA_VERSION)
			continue;

		// If add-on has multiple protocols, also load them
		for (int32 i = 0; i < addOn->CountProtocols(); i++) {
			CayaProtocolAddOn* subAddOn = addOn;
			if (i > 0)
				subAddOn = new CayaProtocolAddOn(id, path.Path(), i);

			CayaProtocol* proto = subAddOn->Protocol();

			fAddOnMap.AddItem(proto->Signature(), subAddOn);
			_LoadAccounts(path.Path(), subAddOn, i, target);
			delete proto;
		}
	}
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


CayaProtocolAddOn*
ProtocolManager::ProtocolAddOnAt(uint32 i) const
{
	return fAddOnMap.ValueAt(i);
}


CayaProtocolAddOn*
ProtocolManager::ProtocolAddOn(const char* signature)
{
	return fAddOnMap.ValueFor(signature);
}


uint32
ProtocolManager::CountProtocolInstances() const
{
	return fProtocolMap.CountItems();
}


CayaProtocol*
ProtocolManager::ProtocolInstanceAt(uint32 i) const
{
	return fProtocolMap.ValueAt(i);
}


CayaProtocol*	
ProtocolManager::ProtocolInstance(bigtime_t identifier)
{
	return fProtocolMap.ValueFor(identifier);
}


void
ProtocolManager::AddAccount(CayaProtocolAddOn* addOn, const char* account,
							BHandler* target)
{
	bigtime_t instanceId = system_time();
	CayaProtocol* cayap = addOn->Protocol();
	(void)new Account(instanceId, cayap, account, addOn->Signature(), target);
	fProtocolMap.AddItem(instanceId, cayap);

	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	theApp->GetMainWindow()->GetServer()->AddProtocolLooper(
		instanceId, cayap);
}


void
ProtocolManager::_LoadAccounts(const char* image_path, CayaProtocolAddOn* addOn,
							   int protoIndex, BHandler* target)
{
	// Find accounts path for this protocol
	BPath path(CayaAccountPath(addOn->Signature(), addOn->Protocol()->Signature()));
	if (path.InitCheck() != B_OK)
		return;

	BDirectory dir(path.Path());
	BEntry entry;
	bool firstDone = false;

	while (dir.GetNextEntry(&entry) == B_OK) {
//		if (firstDone == false) {
			_LoadAccount(addOn, entry, target);
//			firstDone = true;
//		}
//		else
//			_LoadAccount(image_path, entry, protoIndex, target);
	}
}


void
ProtocolManager::_LoadAccount(const char* imagePath, BEntry accountEntry,
							  int protoIndex, BHandler* target)
{
	image_id id = load_add_on(imagePath);
	if (id < 0)
		return;

	// If add-on's API version fits then load accounts...
	CayaProtocolAddOn* addOn = new CayaProtocolAddOn(id, imagePath, protoIndex);
	if (addOn->Version() != CAYA_VERSION)
		return;

	_LoadAccount(addOn, accountEntry, target);
}


void
ProtocolManager::_LoadAccount(CayaProtocolAddOn* addOn, BEntry accountEntry,
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


