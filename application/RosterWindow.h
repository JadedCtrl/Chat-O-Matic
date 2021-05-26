/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */
#ifndef ROSTERWINDOW_H
#define ROSTERWINDOW_H

#include <Window.h>

class RosterItem;
class RosterListView;
class Server;


/* A window with the a list of the user's contacts, will send a message to
   the server with contact info, once a contact is selected. */
class RosterWindow : public BWindow {
public:
	RosterWindow(const char* title, int32 selectMsg, BMessenger* messenger,
		Server* server);

			void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);

			int32		CountItems() const;
			RosterItem*	ItemAt(int index);
			void		AddItem(RosterItem*);
			bool		HasItem(RosterItem*);
			void		RemoveItem(RosterItem*);

			void		UpdateListItem(RosterItem* item);		

private:
	void _PopulateRosterList();

	Server*				fServer;
	RosterListView*		fListView;
	BMessenger*			fTarget;
	BMessage*			fMessage;
};


#endif // ROSTERWINDOW_H

