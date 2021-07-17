/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CONVERSATION_ITEM_H
#define _CONVERSATION_ITEM_H

#include <StringItem.h>

#include "Observer.h"

class Conversation;


class ConversationItem : public BStringItem, public Observer {
public:
	ConversationItem(const char* name, Conversation* chat);

	virtual void	DrawItem(BView* owner, BRect frame, bool complete=false);

	Conversation*	GetConversation();

			void	ObserveString(int32 what, BString str);
			void	ObserveInteger(int32 what, int32 num);

private:
	Conversation* fChat;
	int8 fStatus;
};

#endif // _CONVERSATION_ITEM_H
