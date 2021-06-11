/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONITEM_H
#define CONVERSATIONITEM_H

#include <StringItem.h>

#include "Observer.h"

class Conversation;


class ConversationItem : public BStringItem, public Observer {
public:
	ConversationItem(const char* name, Conversation* chat);

	Conversation* GetConversation();

protected:
	void ObserveString(int32 what, BString str);

private:
	Conversation* fChat;
};


#endif // CONVERSATIONITEM_H
