/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */
#ifndef ROSTERWINDOW_H
#define ROSTERWINDOW_H

#include <Window.h>

#include "Server.h"

class BMenuField;
class RosterItem;
class RosterView;


/* A window with the a list of the user's contacts, will send a message to
   the server with contact info, once a contact is selected. */
class RosterWindow : public BWindow {
public:
	RosterWindow(const char* title, BMessage* selectMsg, BMessenger* messenger,
		Server* server, bigtime_t instance = -1);

			void		MessageReceived(BMessage* message);

			void		UpdateListItem(RosterItem* item);		

private:
	BButton*			fOkButton;
	BMenuField*			fAccountField;
	AccountInstances	fAccounts;

	Server*				fServer;

	RosterView*			fRosterView;
	BMessenger*			fTarget;
	BMessage*			fMessage;
};

#endif // ROSTERWINDOW_H
