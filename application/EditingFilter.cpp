/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <InterfaceDefs.h>
#include <Message.h>
#include <Window.h>

#include "EditingFilter.h"


EditingFilter::EditingFilter(BTextView* view)
	: BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE, B_KEY_DOWN, NULL),
	_view(view)
{
}


filter_result
EditingFilter::Filter(BMessage* message, BHandler** target)
{
	int32 modifiers;

	int8 byte;
	message->FindInt8("byte", &byte);

	// If we have modifiers but none are the Alt key
	if (message->FindInt32("modifiers", &modifiers))
		return B_DISPATCH_MESSAGE;

	// If the Alt key jives with the command_enter status
	if ((modifiers & B_COMMAND_KEY) != 0 && byte == B_ENTER) {
		_view->Insert("\n");
		return B_SKIP_MESSAGE;
	} else if ((modifiers & B_COMMAND_KEY) == 0 && byte == B_ENTER) {
		_view->Window()->PostMessage('chat');
		return B_SKIP_MESSAGE;
	}

	return B_DISPATCH_MESSAGE;
}
