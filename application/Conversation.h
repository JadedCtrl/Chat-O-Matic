/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATION_H
#define CONVERSATION_H

#include <DateTimeFormat.h>
#include <Messenger.h>
#include <Path.h>
#include <StringList.h>

#include "Maps.h"
#include "Notifier.h"
#include "Observer.h"

class BBitmap;
class Contact;
class ConversationItem;
class ConversationView;
class ProtocolLooper;
class Role;
class Server;
class User;


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
	bool				SetNotifyIconBitmap(BBitmap* icon);

	BMessenger			Messenger() const;
	void				SetMessenger(BMessenger messenger);

	ProtocolLooper*		GetProtocolLooper() const;
	void				SetProtocolLooper(ProtocolLooper* looper);

	BString				GetName() const;
	BString				GetSubject() const;

	BBitmap*			ProtocolBitmap() const;
	BBitmap*			IconBitmap() const;

	ConversationView*	GetView();
	void				ShowView(bool typing, bool userAction);
	ConversationItem*	GetListItem();

	UserMap				Users();
	User*				UserById(BString id);
	Contact*			GetOwnContact();

	void				AddUser(User* user);
	void				RemoveUser(User* user);

	void				SetRole(BString id, Role* role);
	Role*				GetRole(BString id);

	int32				GetFlags() { return fRoomFlags; }
	void				SetFlags(int32 flags);
	int32				DisallowedFlags() { return fDisallowedFlags; }

	BPath				CachePath() { return fCachePath; }

private:
	typedef KeyMap<BString, Role*> RoleMap;

	void				_WarnUser(BString message);

	void				_LogChatMessage(BMessage* msg);
	status_t			_GetChatLogs(BMessage* msg);

	void				_CacheRoomFlags();
	void				_LoadRoomFlags();

	void				_EnsureCachePath();
	User*				_EnsureUser(BMessage* msg, bool implicit = true);
	Role*				_GetRole(BMessage* msg);

	void				_UpdateIcon(User* user = NULL);
	bool				_IsDefaultIcon(BBitmap* icon);

	void				_SortConversationList();

	BMessenger	fMessenger;
	ProtocolLooper*	fLooper;
	ConversationView* fChatView;
	ConversationItem* fConversationItem;
	int32 fNotifyMessageCount;
	int32 fNotifyMentionCount;

	BString fID;
	BString fName;
	BString fSubject;

	BBitmap* fIcon;
	bool fUserIcon;

	BPath fCachePath;
	BDateTimeFormat fDateFormatter;

	int32 fRoomFlags;
	int32 fDisallowedFlags;

	UserMap fUsers; // For defined, certain members of the room
	BStringList fGuests; // IDs of implicitly-defined users
	RoleMap fRoles;
};


#endif // CONVERSATION_H

