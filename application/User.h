/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef USER_H
#define USER_H

#include <GraphicsDefs.h>
#include <Message.h>
#include <Messenger.h>
#include <ObjectList.h>
#include <Path.h>
#include <String.h>

#include <libsupport/KeyMap.h>

#include "Notifier.h"
#include "UserStatus.h"

class BBitmap;

class Conversation;
class ProtocolLooper;
class UserPopUp;


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
	UserStatus		GetNotifyStatus() const;
	BString			GetNotifyPersonalStatus() const;

	void			SetNotifyName(BString name);
	void			SetNotifyAvatarBitmap(BBitmap* bitmap);
	void			SetNotifyStatus(UserStatus status);
	void			SetNotifyPersonalStatus(BString personalStatus);

	ChatMap			Conversations();

	rgb_color		fItemColor;

protected:
	virtual void	_EnsureCachePath();

	BBitmap*		_GetCachedAvatar();
	void			_SetCachedAvatar(BBitmap* avatar);

	BMessenger		fMessenger;
	ProtocolLooper*	fLooper;

	BString			fID;
	BString			fName;
	BString			fPersonalStatus;
	BBitmap*		fAvatarBitmap;
	BPath			fCachePath;
	UserStatus		fStatus;
	UserPopUp*		fPopUp;
	ChatMap			fConversations;
};


#endif	// USER_H

