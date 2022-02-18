/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Conversation.h"

#include <Beep.h>
#include <Catalog.h>
#include <DateTimeFormat.h>
#include <Locale.h>
#include <Notification.h>
#include <StringFormat.h>

#include "AppConstants.h"
#include "AppPreferences.h"
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
	fIcon(ImageCache::Get()->GetImage("kOnePersonIcon")),
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

	if (fLooper != NULL)
		fLooper->RemoveConversation(this);

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

			bool winFocus = (win != NULL &&
				(win->IsFront() && !(win->IsMinimized())));
			bool mentioned = ((contact->GetName().IsEmpty() == false
						&& text.IFindFirst(contact->GetName()) != B_ERROR)
					|| (text.IFindFirst(contact->GetId()) != B_ERROR));

			// Sound the bell, if appropriate
			if (winFocus == false) {
				if (mentioned == true
						&& AppPreferences::Get()->SoundOnMention == true)
					system_beep(APP_MENTION_BEEP);
				else if (AppPreferences::Get()->SoundOnMessageReceived == true
						&& ((fUsers.CountItems() <=2 && (fRoomFlags & ROOM_NOTIFY_DM))
							|| (fRoomFlags & ROOM_NOTIFY_ALL)))
					system_beep(APP_MESSAGE_BEEP);
			}

			// Send a notification, if appropriate
			if (winFocus == false && AppPreferences::Get()->NotifyNewMessage) {
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

				if ((fUsers.CountItems() <= 2 && (fRoomFlags & ROOM_NOTIFY_DM))
						|| (fRoomFlags & ROOM_NOTIFY_ALL) || mentioned == true)
				{
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
			}

			// Misc. features Caya contributors planned on adding 
			BWindow* mainWin = ((TheApp*)be_app)->GetMainWindow();
			if (win == NULL && AppPreferences::Get()->MarkUnreadWindow == true)
				mainWin->SetTitle(BString(mainWin->Title()).Prepend("[!]"));

			if (win == NULL && AppPreferences::Get()->MoveToCurrentWorkspace)
				mainWin->SetWorkspaces(B_CURRENT_WORKSPACE);

			if (win == NULL && AppPreferences::Get()->RaiseOnMessageReceived)
				mainWin->Activate(true);


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
				if (name == "me")
					fMessenger.SendMessage(msg);
				else
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
		case IM_ROOM_PARTICIPANTS:
		{
			// Get rid of implicity-defined users rq
			for (int i = 0; i < fGuests.CountStrings(); i++) {
				RemoveUser(UserById(fGuests.StringAt(i)));
				fGuests.Remove(i);
			}

			BStringList ids;
			BStringList names;
			msg->FindStrings("user_name", &names);
			if (msg->FindStrings("user_id", &ids) != B_OK)
				break;

			for (int i = 0; i < ids.CountStrings(); i++) {
				BMessage user;
				user.AddString("user_name", names.StringAt(i));
				user.AddString("user_id", ids.StringAt(i));
				_EnsureUser(&user, false);
			}
			break;
		}
		case IM_ROOM_PARTICIPANT_JOINED:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;

			if (UserById(user_id) == NULL) {
				_EnsureUser(msg, false);
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
		case IM_USER_NICKNAME_SET:
		{
			BString user_id = msg->FindString("user_id");
			BString user_name = msg->FindString("user_name");
			if (user_id.IsEmpty() == false && user_name.IsEmpty() == false) {
				User* user = UserById(user_id);

				BString text(B_TRANSLATE("** %old% has changed their nick to %new%."));
				text.ReplaceAll("%new%", user_name);
				if (user != NULL)
					text.ReplaceAll("%old%", user->GetName());
				else
					text.ReplaceAll("%old%", user_id);

				BMessage* notify = new BMessage(IM_MESSAGE);
				notify->AddInt32("im_what", IM_MESSAGE_RECEIVED);
				notify->AddString("body", text);
				GetView()->MessageReceived(notify);
			}
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
Conversation::SetNotifyIconBitmap(BBitmap* icon)
{
	if (icon != NULL) {
		fIcon = icon;
		NotifyPointer(PTR_ROOM_BITMAP, (void*)icon);
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
	if (user == NULL)
		return;
	BMessage msg;
	msg.AddString("user_id", user->GetId());
	msg.AddString("user_name", user->GetName());
	_EnsureUser(&msg, false);
	_SortConversationList();
}


void
Conversation::RemoveUser(User* user)
{
	if (user == NULL)
		return;
	fUsers.RemoveItemFor(user->GetId());
	user->UnregisterObserver(this);
	GetView()->UpdateUserList(fUsers);
	_SortConversationList();

	_UpdateIcon();
	NotifyInteger(INT_ROOM_MEMBERS, fUsers.CountItems());
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
	if (role != NULL)
		fRoles.AddItem(id, role);
}


Role*
Conversation::GetRole(BString id)
{
	return fRoles.ValueFor(id);
}


void
Conversation::SetFlags(int32 flags)
{
	fRoomFlags = flags;
	_CacheRoomFlags();
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
	// Binary logs
	// TODO: Don't hardcode 31, expose maximum as a setting
	const int32 MAX = 31;

	BMessage logMsg(IM_MESSAGE);
	if (_GetChatLogs(&logMsg) != B_OK) {
		logMsg.what = IM_MESSAGE;
		logMsg.AddInt32("im_what", IM_LOGS_RECEIVED);
	}

	BMessage last;
	if (logMsg.FindMessage("message", MAX, &last) == B_OK)
		logMsg.RemoveData("message", 0);
	msg->AddInt64("when", time(NULL));
	logMsg.AddMessage("message", msg);

	BFile logFile(fCachePath.Path(), B_READ_WRITE | B_OPEN_AT_END | B_CREATE_FILE);
	WriteAttributeMessage(&logFile, "Chat:logs", &logMsg);

	BString mime = BString("text/plain");
	logFile.WriteAttr("BEOS:TYPE", B_MIME_STRING_TYPE, 0, mime.String(),
		mime.CountChars() + 1);

	// Plain-text logs
	// Gotta make sure the formatting's pretty!
	BString date;
	fDateFormatter.Format(date, time(0), B_SHORT_DATE_FORMAT, B_MEDIUM_TIME_FORMAT);
	BString id = msg->FindString("user_id");
	BString name = msg->FindString("user_name");
	BString body = msg->FindString("body");

	if (id.IsEmpty() == true && name.IsEmpty() == true)
		return;
	else if (name.IsEmpty() == true) {
		User* user = UserById(id);
		name = user ? user->GetName() : id;
	}

	BString logLine("[");
	logLine << date << "] <" << name << "> " << body << "\n";
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
	fCachePath = RoomCachePath(fLooper->Protocol()->GetName(), fID.String());
}


User*
Conversation::_EnsureUser(BMessage* msg, bool implicit)
{
	BString id = msg->FindString("user_id");
	BString name = msg->FindString("user_name");
	if (id.IsEmpty() == true) return NULL;

	User* user = UserById(id);
	User* serverUser = fLooper->UserById(id);

	// Not here, but found in server
	if (user == NULL && serverUser != NULL)
		user = serverUser;
	// Not anywhere; create user
	else if (user == NULL) {
		user = new User(id, _GetServer()->Looper());
		user->SetProtocolLooper(fLooper);
		fLooper->AddUser(user);
	}

	// It's been implicitly defined (rather than explicit join), shame!
	if (UserById(id) == NULL && implicit == true) {
		fGuests.Add(id);
		// The response to this will be used to determine if this guest stays
		BMessage msg(IM_MESSAGE);
		msg.AddInt32("im_what", IM_GET_ROOM_PARTICIPANTS);
		msg.AddString("chat_id", fID);
		fLooper->MessageReceived(&msg);
	}

	if (UserById(id) == NULL) {
		fUsers.AddItem(id, user);
		fUsers.AddItem(id, user);
		GetView()->UpdateUserList(fUsers);
		_UpdateIcon(user);
		NotifyInteger(INT_ROOM_MEMBERS, fUsers.CountItems());
	}

	if (name.IsEmpty() == false && user->GetName() != name)
		user->SetNotifyName(name);
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
Conversation::_UpdateIcon(User* user)
{
	if (_IsDefaultIcon(fIcon) == false && fUserIcon == false)
			return;

	// If it's a one-on-one chat, try to use the other user's icon
	if (user != NULL && fUsers.CountItems() == 2
			&& user->GetId() != GetOwnContact()->GetId()
			&& _IsDefaultIcon(user->AvatarBitmap()) == false) {
		fUserIcon = SetNotifyIconBitmap(user->AvatarBitmap());
		return;
	}

	switch (fUsers.CountItems())
	{
		case 0:
		case 1:
			SetNotifyIconBitmap(ImageCache::Get()->GetImage("kOnePersonIcon"));
			break;
		case 2:
			SetNotifyIconBitmap(ImageCache::Get()->GetImage("kTwoPeopleIcon"));
			break;
		case 3:
			SetNotifyIconBitmap(ImageCache::Get()->GetImage("kThreePeopleIcon"));
			break;
		case 4:
			SetNotifyIconBitmap(ImageCache::Get()->GetImage("kFourPeopleIcon"));
			break;
		default:
			SetNotifyIconBitmap(ImageCache::Get()->GetImage("kMorePeopleIcon"));
			break;
	}
	fUserIcon = false;
}


bool
Conversation::_IsDefaultIcon(BBitmap* icon)
{
	return (icon == NULL
			|| icon == ImageCache::Get()->GetImage("kPersonIcon")
			|| icon == ImageCache::Get()->GetImage("kOnePersonIcon")
			|| icon == ImageCache::Get()->GetImage("kTwoPeopleIcon")
			|| icon == ImageCache::Get()->GetImage("kThreePeopleIcon")
			|| icon == ImageCache::Get()->GetImage("kFourPeopleIcon")
			|| icon == ImageCache::Get()->GetImage("kMorePeopleIcon"));
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
