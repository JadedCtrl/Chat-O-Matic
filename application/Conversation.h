/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <DateTimeFormat.h>
#include <Messenger.h>
#include <Path.h>

#include <libsupport/KeyMap.h>

#include "Observer.h"
#include "User.h"

class ChatWindow;
class Contact;
class ConversationItem;
class ConversationView;
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

	ConversationView*	GetView();

	void				ShowView(bool typing, bool userAction);

	BMessenger			Messenger() const;
	void				SetMessenger(BMessenger messenger);

	ProtocolLooper*		GetProtocolLooper() const;
	void				SetProtocolLooper(ProtocolLooper* looper);

	ConversationItem*	GetConversationItem();

	BString				GetName() const;

	UserMap				Users();
	Contact*			UserById(BString id);
	void				AddUser(User* user);

private:
	void				_LogChatMessage(BMessage* msg);
	BStringList			_GetChatLogs();
	void				_EnsureLogPath();

	Contact*			_EnsureUser(BMessage* msg);
	Server*				_GetServer();

	BMessenger	fMessenger;
	ProtocolLooper*	fLooper;
	ConversationView* fChatView;
	ConversationItem* fConversationItem;

	BString fID;
	BString fName;

	BPath fLogPath;
	BDateTimeFormat fDateFormatter;

	UserMap fUsers;
};


#endif // CONVERSATION_H

