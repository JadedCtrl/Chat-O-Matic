/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RoomListRow.h"

#include <ColumnTypes.h>


RoomListRow::RoomListRow(BMessage* msg)
	:
	BRow(),
	fMessage(new BMessage(*msg)),
	fInstance(-1)
{
	int64 proto = msg->FindInt64("instance");
	BString id = msg->FindString("chat_id");
	BString name = msg->GetString("chat_name", id);
	BString desc = msg->FindString("subject");
	int32 user_n = msg->GetInt32("user_count", -1);

	SetField(new BStringField(name), kNameColumn);
	SetField(new BStringField(desc), kDescColumn);
	SetField(new BStringField(id), kIdColumn);
	if (user_n > -1)
		SetField(new BIntegerField(user_n), kUserColumn);
}
