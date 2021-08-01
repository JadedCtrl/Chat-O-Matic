/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "EnterTextView.h"

#include <Window.h>


EnterTextView::EnterTextView(const char* name)
	:
	UrlTextView(name),
	fTarget(NULL)
{
}


EnterTextView::EnterTextView(const char* name, const BFont* initialFont,
	const rgb_color* initialColor, uint32 flags)
	:
	UrlTextView(name, initialFont, initialColor, flags),
	fTarget(NULL)
{
}


void
EnterTextView::KeyDown(const char* bytes, int32 numBytes)
{
	int32 modifiers = Window()->CurrentMessage()->GetInt32("modifiers", 0);

	if (fTarget != NULL && (bytes[0] == B_ENTER) && (modifiers == 0))
	{
		BMessage* msg = new BMessage(fMessage);
		if (fTextSlot.IsEmpty() == false)
			msg->AddString(fTextSlot.String(), Text());
		fTarget->MessageReceived(msg);
	}
	else
		BTextView::KeyDown(bytes, numBytes);
}


void
EnterTextView::SetMessage(BMessage msg, const char* textSlot)
{
	fMessage = msg;
	if (textSlot != NULL)
		fTextSlot.SetTo(textSlot);
}
