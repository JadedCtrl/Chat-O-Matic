/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _SEND_TEXT_VIEW_H
#define _SEND_TEXT_VIEW_H

#include <libinterface/EnterTextView.h>

#include "ConversationView.h"


class SendTextView : public EnterTextView {
public:
	SendTextView(const char* name, ConversationView* convView);

	void KeyDown(const char* bytes, int32 numBytes);

private:
	void _AutoComplete();

	ConversationView* fConversationView;

	int32 fCurrentIndex;
	BString fCurrentWord;
};

#endif // _SEND_TEXT_VIEW_H
