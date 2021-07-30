/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Conversation.h"

#include <Catalog.h>
#include <DateTimeFormat.h>
#include <Locale.h>
#include <Notification.h>
#include <StringFormat.h>
#include <StringList.h>

#include "AppPreferences.h"
#include "Cardie.h"
#include "ChatProtocolMessages.h"
#include "RenderView.h"
#include "ChatCommand.h"
#include "ConversationItem.h"
#include "ConversationView.h"
#include "Flags.h"
#include "ImageCache.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
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
	fIcon(ImageCache::Get()->GetImage("kChatIcon")),
	fDateFormatter(),
	fRoomFlags(0),
	fDisallowedFlags(0),
	fNotifyMessageCount(0),
	fNotifyMentionCount(0),
	fUserIcon(false)
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
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Conversation ― Notifications"

			_EnsureUser(msg);
			_LogChatMessage(msg);
			GetView()->MessageReceived(msg);

			BString text = msg->FindString("body");
			Contact* contact = GetOwnContact();
			BWindow* win = fChatView->Window();

			bool winFocused = (win != NULL &&
				(win->IsFront() && !(win->IsMinimized())));
			bool mentioned = ((text.IFindFirst(contact->GetName()) != B_ERROR)
				|| (text.IFindFirst(contact->GetName()) != B_ERROR));

			// Send a notification, if appropriate
			if (winFocused  == false && AppPreferences::Get()->NotifyNewMessage
				&& (fUsers.CountItems() <= 2 || mentioned == true))
			{
				BString notifyTitle = B_TRANSLATE("New mention");
				BString notifyText = B_TRANSLATE("You've been summoned from "
					"%source%.");

				if (mentioned == false) {
					fNotifyMessageCount++;

					notifyTitle.SetTo(B_TRANSLATE("New message"));
					notifyText.SetTo("");

					BStringFormat pmFormat(B_TRANSLATE("{0, plural,"
						"=1{You've got a new message from %source%.}"
						"other{You've got # new messages from %source%.}}"));
					pmFormat.Format(notifyText, fNotifyMessageCount);
				}
				else
					fNotifyMentionCount++;

				notifyText.ReplaceAll("%source%", GetName());

				BBitmap* icon = IconBitmap();
				if (icon == NULL)
					icon = ProtocolBitmap();


				BNotification notification(B_INFORMATION_NOTIFICATION);
				notification.SetGroup(BString(APP_NAME));
				notification.SetTitle(notifyTitle);
				notification.SetIcon(icon);
				notification.SetContent(notifyText);
				notification.SetMessageID(fID);
				notification.Send();
			}

			// If unattached, highlight the ConversationItem
			if (win == NULL && mentioned == true)
				NotifyInteger(INT_NEW_MENTION, fNotifyMentionCount);
			else if (win == NULL)
				NotifyInteger(INT_NEW_MESSAGE, fNotifyMessageCount);

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
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Conversation ― Command info"

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
				_WarnUser(BString(B_TRANSLATE("That isn't a valid command. "
					"Try /help for a list.")));
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
		case IM_ROOM_ROLECHANGED:
		{
			BString user_id;
			Role* role = _GetRole(msg);
			if (msg->FindString("user_id", &user_id) != B_OK || role == NULL)
				break;

			SetRole(user_id, role);
			GetView()->MessageReceived(msg);
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
	if (what == INT_WINDOW_FOCUSED) {
		fNotifyMessageCount = 0;
		fNotifyMentionCount = 0;
	}
	else
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


bool
Conversation::SetIconBitmap(BBitmap* icon)
{
	if (icon != NULL) {
		fIcon = icon;
		GetView()->UpdateIcon();
		return true;
	}
	return false;
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


BString
Conversation::GetSubject() const
{
	return fSubject;
}


ConversationView*
Conversation::GetView()
{
	if (fChatView != NULL)
		return fChatView;

	fChatView = new ConversationView(this);
	fChatView->RegisterObserver(fConversationItem);
	fChatView->RegisterObserver(this);
	RegisterObserver(fChatView);

	if (!(fRoomFlags & ROOM_POPULATE_LOGS))
		return fChatView;

	BMessage logMsg;
	if (_GetChatLogs(&logMsg) == B_OK)
		fChatView->MessageReceived(&logMsg);

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
	_SortConversationList();
}


void
Conversation::RemoveUser(User* user)
{
	fUsers.RemoveItemFor(user->GetId());
	user->UnregisterObserver(this);
	GetView()->UpdateUserList(fUsers);
	_SortConversationList();
}


Contact*
Conversation::GetOwnContact()
{
	return fLooper->GetOwnContact();
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

		_AdoptUserIcon(user);
	}
	// Not anywhere; create user
	else if (user == NULL) {
		user = new User(id, _GetServer()->Looper());
		user->SetProtocolLooper(fLooper);

		fLooper->AddUser(user);
		fUsers.AddItem(id, user);
		GetView()->UpdateUserList(fUsers);

		_AdoptUserIcon(user);
	}

	if (name.IsEmpty() == false) {
		user->SetNotifyName(name);
	}
	user->RegisterObserver(this);
	return user;
}


Role*
Conversation::_GetRole(BMessage* msg)
{
	if (!msg)
		return NULL;
	BString title;
	int32 perms;
	int32 priority;

	if (msg->FindString("role_title", &title) != B_OK
		|| msg->FindInt32("role_perms", &perms) != B_OK
		|| msg->FindInt32("role_priority", &priority) != B_OK)
		return NULL;

	return new Role(title, perms, priority);
}


void
Conversation::_AdoptUserIcon(User* user)
{
	// If it's a one-on-one chat without custom icon, steal a user's
	if ((fUsers.CountItems() <= 2 && user->GetId() != GetOwnContact()->GetId())
			&& (fIcon == ImageCache::Get()->GetImage("kChatIcon")
				|| fIcon == NULL))
		fUserIcon = SetIconBitmap(user->AvatarBitmap());

	// If it's no longer one-on-one, revert
	if (fUsers.CountItems() > 2 && fUserIcon == true) {
		SetIconBitmap(ImageCache::Get()->GetImage("kChatIcon"));
		fUserIcon = false;
	}
}


void
Conversation::_SortConversationList()
{
	if (fUsers.CountItems() <= 2 || fUsers.CountItems() == 3)
		((TheApp*)be_app)->GetMainWindow()->SortConversation(this);
}


Server*
Conversation::_GetServer()
{
	return ((TheApp*)be_app)->GetMainWindow()->GetServer();
}
