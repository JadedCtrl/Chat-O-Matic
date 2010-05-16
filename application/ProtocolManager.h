/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
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

typedef List<CayaProtocolAddOn*> ProtocolAddOns;
typedef KeyMap<BString, CayaProtocolAddOn*> AddOnMap;
typedef KeyMap<bigtime_t, CayaProtocol*> ProtocolMap;

class ProtocolManager  {
public:
			void				Init(BDirectory dir, BHandler* target);

	static	ProtocolManager*	Get();

			ProtocolAddOns		Protocols();
			ProtocolMap			ProtocolInstances() const;

			CayaProtocol*		ProtocolInstance(bigtime_t identifier);
			CayaProtocolAddOn*	ProtocolAddOn(const char* signature);

			void				AddAccount(CayaProtocolAddOn* addOn,
										   const char* account,
										   BHandler* target);

private:
								ProtocolManager();
			void				_GetAccounts(CayaProtocolAddOn* addOn, BHandler* target);

			AddOnMap			fAddOnMap;
			ProtocolMap			fProtocolMap;
};

#endif	// _PROTOCOL_MANAGER_H
