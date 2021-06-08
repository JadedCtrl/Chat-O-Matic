/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef INVITE_DIALOGUE_H
#define INVITE_DIALOGUE_H

#include <Alert.h>
#include <Messenger.h>

class BBitmap;


// BAlert for quickly sending yes/no messages directly to a protocol looper
class InviteDialogue : public BAlert {
public:
	InviteDialogue(BMessenger target, const char* title, const char* body,
				   BMessage* acceptMsg, BMessage* rejectMsg,
				   BBitmap* icon = NULL);

	void MessageReceived(BMessage* msg);

	status_t Go();

private:
	BMessenger fMessenger;
	BMessage* fAcceptMsg;
	BMessage* fRejectMsg;
};


#endif // INVITE_DIALOGUE_H

