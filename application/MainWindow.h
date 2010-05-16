/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H
 
#include <Window.h>

#include "Observer.h"

class BCardLayout;
class BTextControl;

class Server;
class StatusView;
class RosterListView;
class RosterItem;

class MainWindow: public BWindow, public Observer {
public:
						MainWindow();

			void		Start();

	virtual	void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);
			void		ImError(BMessage* msg);
	virtual	bool		QuitRequested();

			void		ObserveInteger(int32 what, int32 val);

			Server*		GetServer() const { return fServer; }

			void		UpdateListItem(RosterItem* item);		

			int32		CountItems() const;
			RosterItem*	ItemAt(int index);
			void		AddItem(RosterItem*);
			bool		HasItem(RosterItem*);
			void		RemoveItem(RosterItem*);
		
private:
	StatusView*			fStatusView;
	RosterListView*		fListView;
	Server*				fServer;
};

#endif	// _MAIN_WINDOW_H
