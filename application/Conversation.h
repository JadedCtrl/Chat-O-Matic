/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <Messenger.h>
#include <Path.h>

#include <libsupport/KeyMap.h>

#include "Notifier.h"
#include "User.h"

class ChatWindow;
class Contact;
class ProtocolLooper;
class Server;


typedef KeyMap<BString, Contact*> UserMap;


class Conversation : public Observer {
public:
						Conversation(BString id, BMessenger msgn);

	BString				GetId() const;

	// Handles required state changes from an IM message; forwards to ChatWindow
	void				ImMessage(BMessage* msg);

	// Observer inherits; just forwards to ChatWindow
	void				ObserveString(int32 what, BString str);
	void				ObserveInteger(int32 what, int32 value);
	void				ObservePointer(int32 what, void* ptr);

	ChatWindow*			GetChatWindow();
	void 				DeleteWindow();

	void				ShowWindow(bool typing = false, bool userAction = false);
	void				HideWindow();

	BMessenger			Messenger() const;
	void				SetMessenger(BMessenger messenger);

	ProtocolLooper*		GetProtocolLooper() const;
	void				SetProtocolLooper(ProtocolLooper* looper);

	BString				GetName() const;

	UserMap				Users();
	Contact*			UserById(BString id);
	void				AddUser(User* user);

private:
	void				_LogChatMessage(BMessage* msg);
	void				_EnsureLogPath();

	void				_CreateChatWindow();
	Contact*			_EnsureUser(BMessage* msg);
	Server*				_GetServer();

	BMessenger	fMessenger;
	ProtocolLooper*	fLooper;
	ChatWindow* fChatWindow;
	bool fNewWindow;

	BString fID;
	BString fName;

	BPath fLogPath;

	UserMap fUsers;
};


#endif // CONVERSATION_H

