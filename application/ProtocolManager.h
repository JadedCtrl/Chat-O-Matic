/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_MANAGER_H
#define _PROTOCOL_MANAGER_H

#include <Path.h>
#include <String.h>

#include <libsupport/KeyMap.h>

#include "ChatProtocol.h"
#include "ChatProtocolAddOn.h"

class BBitmap;
class BDirectory;
class BHandler;

class MainWindow;
class ProtocolSettings;
class Server;


class ProtocolManager {
public:
			bool				Init(BDirectory dir, BHandler* target);

	static	ProtocolManager*	Get();

			uint32				CountProtocolAddOns() const;
			ChatProtocolAddOn*	ProtocolAddOnAt(uint32 i) const;
			ChatProtocolAddOn*	ProtocolAddOn(const char* signature);

			uint32				CountProtocolInstances() const;
			ChatProtocol*		ProtocolInstanceAt(uint32 i) const;
			ChatProtocol*		ProtocolInstance(bigtime_t identifier);

			void				AddAccount(ChatProtocolAddOn* addOn,
									const char* account, BHandler* target);

			void				EnableAccount(ProtocolSettings* settings,
									const char* account);
			void				DisableAccount(ProtocolSettings* settings,
									const char* account);
			void				ToggleAccount(ProtocolSettings* settings,
									const char* account);

private:
	typedef KeyMap<BString, ChatProtocolAddOn*> AddOnMap;
	typedef KeyMap<bigtime_t, ChatProtocol*> ProtocolMap;

								ProtocolManager();

			void				_LoadAccounts(const char* image_path,
									ChatProtocolAddOn* addOn, int protoIndex,
									BHandler* target);
			void				_LoadAccount(const char* imagePath,
									BEntry accountEntry, int protoIndex,
									BHandler* target);
			void				_LoadAccount(ChatProtocolAddOn* addOn,
									BEntry accountEntry, BHandler* target);

			MainWindow*			_MainWin();

			AddOnMap			fAddOnMap;
			ProtocolMap			fProtocolMap;
};

#endif	// _PROTOCOL_MANAGER_H
