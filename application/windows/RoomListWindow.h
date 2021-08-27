/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ROOM_LIST_WINDOW_H
#define _ROOM_LIST_WINDOW_H

#include <Window.h>

class BButton;
class BColumnListView;
class Server;


class RoomListWindow : public BWindow {
public:
							RoomListWindow(Server* server);
							~RoomListWindow();

	static RoomListWindow*	Get(Server* server);
	static bool				Check();

	virtual void			MessageReceived(BMessage* msg);	

private:
			void			_InitInterface();

	BButton* fJoinButton;
	BColumnListView* fListView;

	Server* fServer;
	static RoomListWindow* fInstance;
};

#endif // _ROOM_LIST_WINDOW_H
