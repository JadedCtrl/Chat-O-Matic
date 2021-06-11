/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONACCITEM_H
#define CONVERSATIONACCITEM_H

#include <StringItem.h>

class Conversation;


class ConversationAccountItem : public BStringItem {
public:
	ConversationAccountItem(const char* name, int64 instance);

	int64 GetInstance();

private:
	int64 fInstance;
};


#endif // CONVERSATIONACCITEM_H

