/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Conversation.h"

#include <DateTimeFormat.h>
#include <Locale.h>
#include <StringList.h>

#include "AppPreferences.h"
#include "ChatProtocolMessages.h"
#include "RenderView.h"
#include "ChatCommand.h"
#include "ConversationItem.h"
#include "ConversationView.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
#include "RoomFlags.h"
#include "Server.h"
#include "TheApp.h"
#include "Utils.h"


Conversation::Conversation(BString id, BMessenger msgn)
	:
	fID(id),
	fName(id),
	fMessenger(msgn),
	fChatView(NULL),
	fLooper(NULL),
	fIcon(NULL),
	fDateFormatter(),
	fRoomFlags(0),
	fDisallowedFlags(0)
{
	fConversationItem = new ConversationItem(fName.String(), this);
	RegisterObserver(fConversationItem);
}


Conversation::~Conversation()
{
	((TheApp*)be_app)->GetMainWindow()->RemoveConversation(this);

	ProtocolLooper* looper = GetProtocolLooper();
	if (looper != NULL)
		looper->RemoveConversation(this);

	delete fChatView;
	delete fConversationItem;
}


BString
Conversation::GetId() const
{
	return fID;
}


void
Conversation::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");

	switch(im_what)
	{
		case IM_MESSAGE_RECEIVED:
		{
			_EnsureUser(msg);
			_LogChatMessage(msg);
			GetView()->MessageReceived(msg);
			break;
		}
		case IM_MESSAGE_SENT:
		{
			_LogChatMessage(msg);
			GetView()->MessageReceived(msg);
			break;
		}
		case IM_SEND_MESSAGE:
		{
			BString body;
			if (msg->FindString("body", &body) != B_OK)
				break;

			if (IsCommand(body.String()) == false) {
				fMessenger.SendMessage(msg);
				break;
			}

			BString name = CommandName(body);
			BString args = CommandArgs(body);
			ChatCommand* cmd = _GetServer()->CommandById(name, fLooper->GetInstance());

			if (cmd == NULL) {
				_WarnUser(BString("That isn't a valid command. Try /help for a list."));
				break;
			}

			BString error("");
			if (cmd->Parse(args, &error, this) == false)
				_WarnUser(error);
			break;
		}
		case IM_ROOM_METADATA:
		{
			BString name;
			if (msg->FindString("chat_name", &name) == B_OK)
				SetNotifyName(name.String());

			BString subject;
			if (msg->FindString("subject", &subject) == B_OK)
				SetNotifySubject(subject.String());

			int32 defaultFlags;
			if (msg->FindInt32("room_default_flags", &defaultFlags) == B_OK)
				if (fRoomFlags == 0)
					fRoomFlags = defaultFlags;

			int32 disabledFlags;
			if (msg->FindInt32("room_disallowed_flags", &disabledFlags) == B_OK)
					fDisallowedFlags = disabledFlags;
			_CacheRoomFlags();
			break;
		}
		case IM_ROOM_PARTICIPANT_JOINED:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;

			if (UserById(user_id) == NULL) {
				_EnsureUser(msg);
				GetView()->MessageReceived(msg);
			}
			break;
		}
		case IM_ROOM_PARTICIPANT_LEFT:
		case IM_ROOM_PARTICIPANT_KICKED:
		case IM_ROOM_PARTICIPANT_BANNED:
		{
			BString user_id = msg->FindString("user_id");
			User* user;
			if (user_id.IsEmpty() == true || (user = UserById(user_id)) == NULL)
				break;

			GetView()->MessageReceived(msg);
			RemoveUser(user);
			break;
		}
		case IM_LOGS_RECEIVED:
		default:
			GetView()->MessageReceived(msg);
	}
}


void
Conversation::ObserveString(int32 what, BString str)
{
	GetView()->InvalidateUserList();
}


void
Conversation::ObserveInteger(int32 what, int32 value)
{
	GetView()->InvalidateUserList();
}


void
Conversation::ObservePointer(int32 what, void* ptr)
{
	GetView()->InvalidateUserList();
}


void
Conversation::SetNotifyName(const char* name)
{
	if (BString(name) == fName)
		return;

	fName = name;
	NotifyString(STR_ROOM_NAME, fName.String());
}


void
Conversation::SetNotifySubject(const char* subject)
{
	if (BString(subject) == fSubject)
		return;

	fSubject = subject;
	NotifyString(STR_ROOM_SUBJECT, fSubject.String());
}


BMessenger
Conversation::Messenger() const
{
	return fMessenger;
}


void
Conversation::SetMessenger(BMessenger messenger)
{
	fMessenger = messenger;
}


ProtocolLooper*
Conversation::GetProtocolLooper() const
{
	return fLooper;
}


void
Conversation::SetProtocolLooper(ProtocolLooper* looper)
{
	fLooper = looper;
	_LoadRoomFlags();
}


BBitmap*
Conversation::ProtocolBitmap() const
{
	ChatProtocol* protocol = fLooper->Protocol();
	ChatProtocolAddOn* addOn
		= ProtocolManager::Get()->ProtocolAddOn(protocol->Signature());

	return addOn->ProtoIcon();
}


BBitmap*
Conversation::IconBitmap() const
{
	return fIcon;
}


BString
Conversation::GetName() const
{
	return fName;
}


ConversationView*
Conversation::GetView()
{
	if (fChatView != NULL)
		return fChatView;

	fChatView = new ConversationView(this);

	if (!(fRoomFlags & ROOM_POPULATE_LOGS))
		return fChatView;

	BMessage logMsg;
	if (_GetChatLogs(&logMsg) == B_OK)
		fChatView->MessageReceived(&logMsg);

	RegisterObserver(fChatView);
	return fChatView;
}


void
Conversation::ShowView(bool typing, bool userAction)
{
	((TheApp*)be_app)->GetMainWindow()->SetConversation(this);
}


ConversationItem*
Conversation::GetListItem()
{
	return fConversationItem;
}


UserMap
Conversation::Users()
{
	return fUsers;
}


User*
Conversation::UserById(BString id)
{
	bool found = false;
	return fUsers.ValueFor(id, &found);
}


void
Conversation::AddUser(User* user)
{
	BMessage msg;
	msg.AddString("user_id", user->GetId());
	msg.AddString("user_name", user->GetName());
	_EnsureUser(&msg);
}


void
Conversation::RemoveUser(User* user)
{
	fUsers.RemoveItemFor(user->GetId());
	user->UnregisterObserver(this);
	GetView()->UpdateUserList(fUsers);
}


BString
Conversation::GetOwnId()
{
	return fLooper->GetOwnId();
}


void
Conversation::SetRole(BString id, Role* role)
{
	Role* oldRole = fRoles.ValueFor(id);
	if (oldRole != NULL) {
		fRoles.RemoveItemFor(id);
		delete oldRole;
	}

	fRoles.AddItem(id, role);
}


Role*
Conversation::GetRole(BString id)
{
	return fRoles.ValueFor(id);
}


void
Conversation::_WarnUser(BString message)
{
	BMessage* warning = new BMessage(IM_MESSAGE);
	warning->AddInt32("im_what", IM_MESSAGE_RECEIVED);
	warning->AddString("body", message.Append('\n', 1).InsertChars("-- ", 0));
	GetView()->MessageReceived(warning);
}


void
Conversation::_LogChatMessage(BMessage* msg)
{
	BString date;
	fDateFormatter.Format(date, time(0), B_SHORT_DATE_FORMAT, B_MEDIUM_TIME_FORMAT);

	BString id = msg->FindString("user_id");
	BString body = msg->FindString("body");

	if (id.IsEmpty() == true)
		return;

	// Binary logs
	// TODO: Don't hardcode 21, expose maximum as a setting
	BStringList users, bodies;
	int64 times[21] = { 0 };
	times[0] = (int64)time(NULL);

	BMessage logMsg;
	if (_GetChatLogs(&logMsg) == B_OK) {
		logMsg.FindStrings("body", &bodies);
		logMsg.FindStrings("user_id", &users);

		int64 found;
		for (int i = 0; i < 21; i++)
			if (logMsg.FindInt64("when", i, &found) == B_OK)
				times[i + 1] = found;

		bodies.Remove(21);
		users.Remove(21);
		bodies.Add(body, 0);
		users.Add(id, 0);
	}

	BMessage newLogMsg(IM_MESSAGE);
	newLogMsg.AddInt32("im_what", IM_LOGS_RECEIVED);
	newLogMsg.AddStrings("body", bodies);
	newLogMsg.AddStrings("user_id", users);
	newLogMsg.AddInt64("when", time(NULL));
	for (int i = 0; i < 21; i++)
		newLogMsg.AddInt64("when", times[i]);

	BFile logFile(fCachePath.Path(), B_READ_WRITE | B_OPEN_AT_END | B_CREATE_FILE);
	WriteAttributeMessage(&logFile, "Chat:logs", &newLogMsg);

	// Plain-text logs
	BString uname;
	if (id.IsEmpty() == false)
		uname = UserById(id)->GetName();
	else
		uname = "You";

	BString logLine("[");
	logLine << date << "] <" << uname << "> " << body << "\n";

	logFile.Write(logLine.String(), logLine.Length());
}


status_t
Conversation::_GetChatLogs(BMessage* msg)
{
	_EnsureCachePath();

	BFile logFile(fCachePath.Path(), B_READ_WRITE | B_CREATE_FILE);

	return ReadAttributeMessage(&logFile, "Chat:logs", msg);
}


void
Conversation::_CacheRoomFlags()
{
	_EnsureCachePath();
	BFile cacheFile(fCachePath.Path(), B_READ_WRITE | B_CREATE_FILE);
	if (cacheFile.InitCheck() != B_OK)
		return;

	cacheFile.WriteAttr("Chat:flags", B_INT32_TYPE, 0, &fRoomFlags, sizeof(int32));
}


void
Conversation::_LoadRoomFlags()
{
	_EnsureCachePath();
	BFile cacheFile(fCachePath.Path(), B_READ_ONLY);
	if (cacheFile.InitCheck() != B_OK)
		return;

	cacheFile.ReadAttr("Chat:flags", B_INT32_TYPE, 0, &fRoomFlags, sizeof(int32));
}


void
Conversation::_EnsureCachePath()
{
	if (fCachePath.InitCheck() == B_OK)
		return;
	fCachePath.SetTo(RoomCachePath(fLooper->Protocol()->GetName(),
									   fID.String()));
}


User*
Conversation::_EnsureUser(BMessage* msg)
{
	BString id = msg->FindString("user_id");
	BString name = msg->FindString("user_name");
	if (id.IsEmpty() == true) return NULL;

	User* user = UserById(id);
	User* serverUser = fLooper->UserById(id);

	// Not here, but found in server
	if (user == NULL && serverUser != NULL) {
		fUsers.AddItem(id, serverUser);
		user = serverUser;
		GetView()->UpdateUserList(fUsers);
	}
	// Not anywhere; create user
	else if (user == NULL) {
		user = new User(id, _GetServer()->Looper());
		user->SetProtocolLooper(fLooper);

		fLooper->AddUser(user);
		fUsers.AddItem(id, user);
		GetView()->UpdateUserList(fUsers);
	}

	if (name.IsEmpty() == false) {
		user->SetNotifyName(name);
	}
	user->RegisterObserver(this);
	return user;
}


Server*
Conversation::_GetServer()
{
	return ((TheApp*)be_app)->GetMainWindow()->GetServer();
}


