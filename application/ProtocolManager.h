/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef ProtocolManager_h
#define ProtocolManager_h

#include <Directory.h>
#include <Path.h>
#include <String.h>

#include "KeyMap.h"
#include "CayaProtocol.h"

class BBitmap;

class ProtocolManager 
{
public:
						void				Init(BDirectory protocolDir);
				static	ProtocolManager*	Get();
					
		CayaProtocol*	GetProtocol(BString signature);

		BList*			GetProtocols();

		BPath*			GetProtocolPath(BString signature);
		BBitmap*		GetProtocolIcon(BString signature);
	
	private:
		
		ProtocolManager();
		
		KeyMap<BString, CayaProtocol*>	fProtocolMap;
		KeyMap<BString, BPath*>			fAddonMap;
};
#endif
