/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SERVER_H
#define _SERVER_H
 
#include <Message.h>
#include <MessageFilter.h>

#include "CayaConstants.h"
#include "ContactLinker.h"
#include "KeyMap.h"

class MainWindow;
class RosterItem;
class LooperCayaProtocol;

typedef KeyMap<BString, ContactLinker*> RosterMap;

class Server: public BMessageFilter {
public:
							Server(MainWindow* mainWindow);

	virtual	filter_result	Filter(BMessage* message, BHandler** target);
			filter_result	ImMessage(BMessage* msg);

			void			UpdateSettings(BMessage settings);

			void			Login();

			void			SendProtocolMessage(BMessage* msg);
			void			SendChatMessage(BMessage* msg);

			RosterMap		RosterItems() const;
			RosterItem*		RosterItemForId(BString id);

			void			Quit();
			
			//TODO: there should be a contact for each account.
			ContactLinker*	GetOwnContact();

private:
	ContactLinker*			EnsureContactLinker(BString id);

	RosterMap				fRosterMap;
	MainWindow*				fMainWindow;
	LooperCayaProtocol*		fProtocol;
	ContactLinker*			fMySelf;
};

#endif	// _SERVER_H
