/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <stdio.h>

#include <Entry.h>
#include <image.h>

#include <libinterface/BitmapUtils.h>

#include "ProtocolManager.h"
#include "CayaProtocol.h"
#include "CayaUtils.h"

static ProtocolManager*	fInstance = NULL;


void
ProtocolManager::Init(BDirectory protocolDir)
{
	BEntry entry;
	BPath path;

	protocolDir.Rewind();

	while (protocolDir.GetNextEntry(&entry) == B_OK) {
		path = BPath(&entry);

		image_id id = load_add_on(path.Path());
		if (id >= 0) {
			CayaProtocol* (*main_protocol)();
			if (get_image_symbol(id, "main_protocol", B_SYMBOL_TYPE_TEXT,
				(void**)&main_protocol) == B_OK) {
				CayaProtocol* cayp = (*main_protocol)();

				if (cayp) {
					printf("Found a new Protocol: %s [%s]\n", cayp->GetFriendlySignature(),
						cayp->GetSignature());
					fProtocolMap.AddItem(BString(cayp->GetSignature()), cayp);
					fAddonMap.AddItem(BString(cayp->GetSignature()), new BPath(path));
				} else
					unload_add_on(id);
			} else
				unload_add_on(id);
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


CayaProtocol*	
ProtocolManager::GetProtocol(BString signature)
{
	return fProtocolMap.ValueFor(signature);
}


BList*
ProtocolManager::GetProtocols()
{
	return fProtocolMap.Items();
}


BPath*
ProtocolManager::GetProtocolPath(BString signature)
{
	return fAddonMap.ValueFor(signature);
}


BBitmap*
ProtocolManager::GetProtocolIcon(BString signature)
{
	BPath* path = fAddonMap.ValueFor(signature);
	return ReadNodeIcon(path->Path(), B_LARGE_ICON, true);
}
