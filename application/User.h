/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef USER_H
#define USER_H

#include <String.h>
#include <Message.h>
#include <Messenger.h>
#include <ObjectList.h>

#include <libsupport/KeyMap.h>

#include "Notifier.h"
#include "CayaConstants.h"

class BBitmap;

class ChatWindow;
class Conversation;
class UserPopUp;
class ProtocolLooper;
class RosterItem;


typedef KeyMap<BString, Conversation*> ChatMap;


class User : public Notifier {
public:
					User(BString id, BMessenger msgn);

	void			RegisterObserver(Conversation* chat);
	void			RegisterObserver(Observer* obs) { Notifier::RegisterObserver(obs); }
	void			UnregisterObserver(Conversation* chat);
	void			UnregisterObserver(Observer* obs) { Notifier::UnregisterObserver(obs); }

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

	ChatMap			Conversations();

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
	ChatMap			fConversations;
};


#endif	// USER_H

