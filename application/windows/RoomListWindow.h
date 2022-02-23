/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ROOM_LIST_WINDOW_H
#define _ROOM_LIST_WINDOW_H

#include <ObjectList.h>
#include <Window.h>

#include <libsupport/KeyMap.h>

class BButton;
class BColumnListView;
class RoomListRow;


typedef KeyMap<int64, BObjectList<RoomListRow>*> RowMap;


class RoomListWindow : public BWindow {
public:
							RoomListWindow();
							~RoomListWindow();

	static RoomListWindow*	Get();
	static bool				Check();

	virtual void			MessageReceived(BMessage* msg);	

private:
			void			_InitInterface();

			void			_EmptyList();

	BButton* fJoinButton;
	BColumnListView* fListView;

	RowMap fRows;
	int64 fAccount;

	static RoomListWindow* fInstance;
};

#endif // _ROOM_LIST_WINDOW_H
