/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONITEM_H
#define CONVERSATIONITEM_H

#include <ListView.h>

#include "Observer.h"

class Conversation;


class ConversationItem : public BStringItem, public Observer {
public:
	ConversationItem(const char* name, Conversation* chat);

	Conversation* GetConversation();

protected:
	void ObserveString(int32 what, BString str);
	void ObservePointer(int32 what, void* ptr);
	void ObserveInteger(int32 what, int32 val);

private:
	Conversation* fChat;
};


#endif // CONVERSATIONITEM_H
