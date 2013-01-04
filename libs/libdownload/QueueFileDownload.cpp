/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "QueueFileDownload.h"
#include "ActionDownload.h"
#include "Messenger.h"

void
QueueFileDownload::ActionReadyToPerform(Action* action)
{
	ActionDownload* ad = dynamic_cast<ActionDownload*>(action);
	if (!ad) 
		return;

	ad->SetLooper(fLooper, fMsg);

}


void
QueueFileDownload::ActionPerformed(Action* action, status_t state, BMessage* msg)
{

	ActionDownload* ad = dynamic_cast<ActionDownload*>(action);
	if (!ad) 
		return;

	BMessage notify(*msg);
	notify.what = fMsg;

	if (fLooper)
		BMessenger(fLooper).SendMessage(&notify);

	return;
}


void
QueueFileDownload::SuppressAction(Action* action)
{
	ActionDownload* ad = dynamic_cast<ActionDownload*>(action);
	if (!ad) 
		return;

	ad->SetShouldStop(true);
}

//--

