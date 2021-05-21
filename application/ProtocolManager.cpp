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

		fAddOnMap.AddItem(addOn->Signature(), addOn);
		_GetAccounts(addOn, addOn->Signature(), target);

		// If add-on has multiple protocols, also load them
		for (int32 i = 1; i < addOn->CountProtocols(); i++) {
			CayaProtocolAddOn* subAddOn =
				new CayaProtocolAddOn(id,path.Path(), i);
			CayaProtocol* proto = subAddOn->Protocol();

			fAddOnMap.AddItem(proto->Signature(), subAddOn);
			_GetAccounts(subAddOn, proto->Signature(), target);
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
	(void)new Account(instanceId, cayap, account, target);
	fProtocolMap.AddItem(instanceId, cayap);

	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	theApp->GetMainWindow()->GetServer()->AddProtocolLooper(
		instanceId, cayap);
}


void
ProtocolManager::_GetAccounts(CayaProtocolAddOn* addOn, const char* subProtocol,
								BHandler* target)
{
	// Find accounts path for this protocol
	BPath path(CayaAccountPath(addOn->Signature(), subProtocol));
	if (path.InitCheck() != B_OK)
		return;

	BDirectory dir(path.Path());
	BEntry entry;
	while (dir.GetNextEntry(&entry) == B_OK) {
		BFile file(&entry, B_READ_ONLY);
		BMessage msg;

		if (msg.Unflatten(&file) == B_OK) {
			char buffer[B_PATH_NAME_LENGTH];
			if (entry.GetName(buffer) == B_OK) {
				printf("Found %s for protocol %s!\n", buffer, addOn->Signature());
				AddAccount(addOn, buffer, target);
			}
		}
	}
}


