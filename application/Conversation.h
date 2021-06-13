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
#include "Role.h"
#include "Server.h"
#include "User.h"

class BBitmap;
class ConversationItem;
class ConversationView;
class ProtocolLooper;
class Server;


typedef KeyMap<BString, User*> UserMap;
typedef KeyMap<BString, Role*> RoleMap;


class Conversation : public Notifier, public Observer {
public:
						Conversation(BString id, BMessenger msgn);
						~Conversation();

	BString				GetId() const;

	void				ImMessage(BMessage* msg);

	// Tell the ConversationView to invalidate user list
	void				ObserveString(int32 what, BString str);
	void				ObserveInteger(int32 what, int32 value);
	void				ObservePointer(int32 what, void* ptr);

	void				SetNotifyName(const char* name);
	void				SetNotifySubject(const char* subject);

	BMessenger			Messenger() const;
	void				SetMessenger(BMessenger messenger);

	ProtocolLooper*		GetProtocolLooper() const;
	void				SetProtocolLooper(ProtocolLooper* looper);

	BBitmap*			ProtocolBitmap() const;
	BBitmap*			IconBitmap() const;
	BString				GetName() const;

	ConversationView*	GetView();
	void				ShowView(bool typing, bool userAction);
	ConversationItem*	GetListItem();

	UserMap				Users();
	User*				UserById(BString id);
	BString				GetOwnId();

	void				AddUser(User* user);
	void				RemoveUser(User* user);

	void				SetRole(BString id, Role* role);
	Role*				GetRole(BString id);

private:
	void				_LogChatMessage(BMessage* msg);
	status_t			_GetChatLogs(BMessage* msg);

	void				_EnsureCachePath();

	User*				_EnsureUser(BMessage* msg);

	BMessenger	fMessenger;
	ProtocolLooper*	fLooper;
	ConversationView* fChatView;
	ConversationItem* fConversationItem;

	BString fID;
	BString fName;
	BString fSubject;

	BBitmap* fIcon;

	BPath fCachePath;
	BDateTimeFormat fDateFormatter;

	int32 fRoomFlags;
	int32 fDisallowedFlags;

	UserMap fUsers;
	RoleMap fRoles;
};


#endif // CONVERSATION_H

