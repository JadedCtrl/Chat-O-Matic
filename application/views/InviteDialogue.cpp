/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "InviteDialogue.h"

#include <Messenger.h>


InviteDialogue::InviteDialogue(BMessenger target, const char* title,
	const char* body, BMessage* acceptMsg, BMessage* rejectMsg, BBitmap* icon)
	:
	BAlert(title, body, "Cancel", "Reject", "Accept", B_WIDTH_AS_USUAL,
		   B_OFFSET_SPACING),
	fMessenger(target),
	fAcceptMsg(acceptMsg),
	fRejectMsg(rejectMsg)
{
	if (icon != NULL)
		SetIcon(icon);
}


void
InviteDialogue::MessageReceived(BMessage* msg)
{
	int32 which;
	if (msg->FindInt32("which", &which) != B_OK) {
		BAlert::MessageReceived(msg);
		return;
	}
	msg->PrintToStream();

	switch (which)
	{
		case 0:
			break;
		case 1:
			fMessenger.SendMessage(fRejectMsg);
			break;
		case 2:
			fMessenger.SendMessage(fAcceptMsg);
			break;
		default:
			return;
	}

	PostMessage(B_QUIT_REQUESTED);
}


status_t
InviteDialogue::Go()
{
	return BAlert::Go(NULL);
}


