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
#ifndef _ROSTER_VIEW_H
#define _ROSTER_VIEW_H

#include <GroupView.h>

#include "Server.h"

class BTextControl;
class RosterItem;
class RosterListView;
class Server;


class RosterView : public BGroupView {
public:
	RosterView(const char* title, Server* server, bigtime_t account = -1);

			void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);

			void		AttachedToWindow();
			void		SetInvocationMessage(BMessage* msg);

			void		SetAccount(bigtime_t instance_id);
			
			void		UpdateListItem(RosterItem* item);		

		RosterListView*	ListView();

private:
			RosterMap	_RosterMap();

	Server*				fServer;
	RosterListView*		fListView;
	BTextControl*		fSearchBox;
	bigtime_t			fAccount;
};

#endif // _ROSTER_VIEW_H
