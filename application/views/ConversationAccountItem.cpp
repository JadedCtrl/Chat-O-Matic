/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationAccountItem.h"

#include "ProtocolLooper.h"


ConversationAccountItem::ConversationAccountItem(const char* name,
	int64 instance, ProtocolLooper* looper)
	:
	BStringItem(name),
	fInstance(instance),
	fProtocolLooper(looper)
{
}


int64
ConversationAccountItem::GetInstance()
{
	return fInstance;
}


ProtocolLooper*
ConversationAccountItem::GetLooper()
{
	return fProtocolLooper;
}
