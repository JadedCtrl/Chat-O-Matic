/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationItem.h"

#include <InterfaceDefs.h>
#include <View.h>

#include "Conversation.h"
#include "NotifyMessage.h"
#include "Utils.h"


const int8 kMentioned = 1;
const int8 kMessage = 2;


ConversationItem::ConversationItem(const char* name, Conversation* chat)
	:
	BStringItem(name),
	fChat(chat),
	fStatus(0)
{
}


void
ConversationItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	rgb_color old = owner->HighColor();

	if (fStatus & kMentioned)
		owner->SetHighUIColor(B_SUCCESS_COLOR);
	else if (fStatus & kMessage)
		owner->SetHighColor(TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 2));

	BStringItem::DrawItem(owner, frame, complete);
	owner->SetHighColor(old);
}


Conversation*
ConversationItem::GetConversation()
{
	return fChat;
}


void 
ConversationItem::ObserveString(int32 what, BString str)
{
	switch (what)
	{
		case STR_ROOM_NAME:
			SetText(str.String());
			break;
	}
}


void
ConversationItem::ObserveInteger(int32 what, int32 num)
{
	switch (what)
	{
		case INT_NEW_MESSAGE:
			fStatus |= kMessage;
			break;
		case INT_NEW_MENTION:
			fStatus |= kMentioned;
			break;
		case INT_WINDOW_FOCUSED:
			fStatus = 0;
			break;
	}
}
