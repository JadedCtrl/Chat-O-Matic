/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SendTextView.h"

#include <Window.h>

#include "AppMessages.h"


SendTextView::SendTextView(const char* name, ConversationView* convView)
	:
	BTextView(name),
	fConversationView(convView)
{
}


void
SendTextView::KeyDown(const char* bytes, int32 numBytes)
{
	int32 modifiers = Window()->CurrentMessage()->GetInt32("modifiers", 0);

	if ((bytes[0] == B_ENTER) && (modifiers & B_COMMAND_KEY))
		Insert("\n");
	else if (bytes[0] == B_ENTER)
		fConversationView->MessageReceived(new BMessage(APP_CHAT));
	else
		BTextView::KeyDown(bytes, numBytes);
}
