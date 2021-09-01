/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ROOM_LIST_ROW_H
#define _ROOM_LIST_ROW_H

#include <ColumnListView.h>


enum {
	kNameColumn,
	kDescColumn,
	kCatColumn,
	kUserColumn
};


class RoomListRow : public BRow {
public:
				RoomListRow(BMessage* msg);
				~RoomListRow();

	BMessage*	Message() { return fMessage; }
	int64		Instance() { return fInstance; }

private:
	int64 fInstance;
	BMessage* fMessage;
};

#endif // _ROOM_LIST_ROW_H
