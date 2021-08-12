/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONACCITEM_H
#define CONVERSATIONACCITEM_H

#include <StringItem.h>

class Conversation;
class ProtocolLooper;


class ConversationAccountItem : public BStringItem {
public:
	ConversationAccountItem(const char* name, int64 instance,
		ProtocolLooper* looper);

	int64 GetInstance();
	ProtocolLooper* GetLooper();

private:
	int64 fInstance;
	ProtocolLooper* fProtocolLooper;
};


#endif // CONVERSATIONACCITEM_H

