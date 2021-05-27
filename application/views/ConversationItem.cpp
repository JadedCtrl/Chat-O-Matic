/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationItem.h"

#include "Conversation.h"


ConversationItem::ConversationItem(const char* name, Conversation* chat)
	:
	BStringItem(name),
	fChat(chat)
{
}


Conversation*
ConversationItem::GetConversation()
{
	return fChat;
}


void 
ConversationItem::ObserveString(int32 what, BString str)
{
}


void 
ConversationItem::ObservePointer(int32 what, void* ptr)
{
}


void 
ConversationItem::ObserveInteger(int32 what, int32 val)
{
}


