/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2009, Michael Davidson. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Messenger.h>

#include "NotifyingTextView.h"


NotifyingTextView::NotifyingTextView(const char* name, uint32 flags)
	: BTextView(name, flags),
	fMessenger(NULL)
{
}


NotifyingTextView::~NotifyingTextView()
{
	if (fMessenger != NULL)
	delete fMessenger;
}


void
NotifyingTextView::SetTarget(const BHandler* handler)
{
	if (fMessenger != NULL)
		delete fMessenger;
	fMessenger = new BMessenger(handler);
}


BMessage*
NotifyingTextView::Message() const
{
	return fMessage;
}


void
NotifyingTextView::SetMessage(BMessage* msg)
{
	fMessage = msg;
}
		

void
NotifyingTextView::InsertText(const char* text, int32 length, int32 offset,
	const text_run_array* runs)
{
	if ((fMessenger != NULL) && fMessenger->IsValid()) {
		BMessage msg(*fMessage);
		msg.AddPointer("source", this);
		fMessenger->SendMessage(&msg);
	}

	BTextView::InsertText(text, length, offset, runs);
}


void
NotifyingTextView::DeleteText(int32 start, int32 finish)
{
	if ((fMessenger != NULL) && fMessenger->IsValid()) {
		BMessage msg(*fMessage);
		msg.AddPointer("source", this);
		fMessenger->SendMessage(&msg);
	}

	BTextView::DeleteText(start, finish);
}
