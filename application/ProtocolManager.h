/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_MANAGER_H
#define _PROTOCOL_MANAGER_H

#include <Path.h>
#include <String.h>

#include <libsupport/KeyMap.h>

#include "CayaProtocol.h"
#include "CayaProtocolAddOn.h"

class BBitmap;
class BDirectory;
class BHandler;

class ProtocolManager {
public:
			void				Init(BDirectory dir, BHandler* target);

	static	ProtocolManager*	Get();

			uint32				CountProtocolAddOns() const;
			CayaProtocolAddOn*	ProtocolAddOnAt(uint32 i) const;
			CayaProtocolAddOn*	ProtocolAddOn(const char* signature);

			uint32				CountProtocolInstances() const;
			CayaProtocol*		ProtocolInstanceAt(uint32 i) const;
			CayaProtocol*		ProtocolInstance(bigtime_t identifier);

			void				AddAccount(CayaProtocolAddOn* addOn,
									const char* account,
									BHandler* target);

private:
	typedef KeyMap<BString, CayaProtocolAddOn*> AddOnMap;
	typedef KeyMap<bigtime_t, CayaProtocol*> ProtocolMap;

								ProtocolManager();
			void				_GetAccounts(CayaProtocolAddOn* addOn,
									BHandler* target);

			AddOnMap			fAddOnMap;
			ProtocolMap			fProtocolMap;
};

#endif	// _PROTOCOL_MANAGER_H
