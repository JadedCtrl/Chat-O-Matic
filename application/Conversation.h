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
#include "Server.h"
#include "User.h"

class ConversationItem;
class ConversationView;
class ProtocolLooper;
class Server;


typedef KeyMap<BString, User*> UserMap;


class Conversation : public Notifier, public Observer {
public:
						Conversation(BString id, BMessenger msgn);

	BString				GetId() const;

	void				ImMessage(BMessage* msg);

	// Tell the ConversationView to invalidate user list
	void				ObserveString(int32 what, BString str);
	void				ObserveInteger(int32 what, int32 value);
	void				ObservePointer(int32 what, void* ptr);

	BMessenger			Messenger() const;
	void				SetMessenger(BMessenger messenger);

	ProtocolLooper*		GetProtocolLooper() const;
	void				SetProtocolLooper(ProtocolLooper* looper);

	void				ShowView(bool typing, bool userAction);

	ConversationView*	GetView();
	ConversationItem*	GetListItem();

	BString				GetName() const;

	UserMap				Users();
	User*				UserById(BString id);

	void				AddUser(User* user);
	void				RemoveUser(User* user);

private:
	void				_LogChatMessage(BMessage* msg);
	BStringList			_GetChatLogs();
	void				_EnsureLogPath();

	User*			_EnsureUser(BMessage* msg);
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

