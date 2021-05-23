/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_LINKER_H_
#define _CONTACT_LINKER_H_

#include <String.h>
#include <Message.h>
#include <Messenger.h>

#include "Notifier.h"
#include "CayaConstants.h"

class BBitmap;

class ChatWindow;
class ContactPopUp;
class ProtocolLooper;
class RosterItem;

class Contact : public Notifier {
public:
					Contact(BString id, BMessenger msgn);

	ChatWindow*		GetChatWindow();
	void 			DeleteWindow();

	void			ShowWindow(bool typing = false, bool userAction = false);
	void			HideWindow();

	void			ShowPopUp(BPoint where);
	void			DeletePopUp();
	void			HidePopUp();

	RosterItem*		GetRosterItem() const;

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

private:
	void			CreateChatWindow();

	RosterItem*		fRosterItem;
	ChatWindow*		fChatWindow;
	BMessenger		fMessenger;
	ProtocolLooper*	fLooper;

	BString			fID;
	bigtime_t		fInstance;
	BString			fName;
	BString			fPersonalStatus;
	BBitmap*		fAvatarBitmap;
	CayaStatus		fStatus;
	ContactPopUp*	fPopUp;

	bool			fNewWindow;
};

#endif	// _CONTACT_LINKER_H_
