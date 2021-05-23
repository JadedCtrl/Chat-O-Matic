/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _USER_H_
#define _USER_H_

#include <String.h>
#include <Message.h>
#include <Messenger.h>

#include "Notifier.h"
#include "CayaConstants.h"

class BBitmap;

class ChatWindow;
class UserPopUp;
class ProtocolLooper;
class RosterItem;

class User : public Notifier {
public:
					User(BString id, BMessenger msgn);

	void			ShowPopUp(BPoint where);
	void			DeletePopUp();
	void			HidePopUp();

	BString			GetId() const;

	BMessenger		Messenger() const;
	void			SetMessenger(BMessenger messenger);

	ProtocolLooper*	GetProtocolLooper() const;
	void			SetProtocolLooper(ProtocolLooper* looper);
	BBitmap*		ProtocolBitmap() const;

	BString			GetName() const;
	BBitmap*		AvatarBitmap() const;
	CayaStatus		GetNotifyStatus() const;
	BString			GetNotifyPersonalStatus() const;

	void			SetNotifyName(BString name);
	void			SetNotifyAvatarBitmap(BBitmap* bitmap);
	void			SetNotifyStatus(CayaStatus status);
	void			SetNotifyPersonalStatus(BString personalStatus);

protected:
	BMessenger		fMessenger;
	ProtocolLooper*	fLooper;

	BString			fID;
	bigtime_t		fInstance;
	BString			fName;
	BString			fPersonalStatus;
	BBitmap*		fAvatarBitmap;
	CayaStatus		fStatus;
	UserPopUp*		fPopUp;
};

#endif	// _USER_H_
