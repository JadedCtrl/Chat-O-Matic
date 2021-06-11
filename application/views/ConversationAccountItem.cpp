/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationAccountItem.h"


ConversationAccountItem::ConversationAccountItem(const char* name, int64 instance)
	:
	BStringItem(name),
	fInstance(instance)
{
}


int64
ConversationAccountItem::GetInstance()
{
	return fInstance;
}


