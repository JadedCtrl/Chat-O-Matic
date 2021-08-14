/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 *
 * Contributors:
 *		Dario Casalinuovo
 */

#include "Server.h"

#include <Application.h>
#include <Catalog.h>
#include <Debug.h>
#include <Directory.h>
#include <Entry.h>
#include <Notification.h>
#include <Path.h>
#include <StringList.h>
#include <TranslationUtils.h>

#include "Account.h"
#include "AccountManager.h"
#include "AppMessages.h"
#include "AppPreferences.h"
#include "Cardie.h"
#include "ChatProtocol.h"
#include "ConversationInfoWindow.h"
#include "ConversationView.h"
#include "ChatProtocolMessages.h"
#include "Flags.h"
#include "ImageCache.h"
#include "InviteDialogue.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
#include "RosterItem.h"
#include "UserInfoWindow.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Server"


Server::Server()
	:
	BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE)
{
	if (fChatItems.IsEmpty() == false || fUserItems.IsEmpty() == false
			|| fCommands.CountItems() > 0)
		return;

	BResources res = ChatResources();
	if (res.InitCheck() != B_OK)
		return;

	// Loading user pop-up items
	for (int i = 0; i < 6; i++) {
		size_t size;
		BMessage temp;
		const void* buff = res.LoadResource(B_MESSAGE_TYPE, 1100 + i, &size);
		temp.Unflatten((const char*)buff);
		fUserItems.AddItem(new BMessage(temp));
	}

	// Loading room pop-up items
	for (int i = 0; i < 2; i++) {
		size_t size;
		BMessage temp;
		const void* buff = res.LoadResource(B_MESSAGE_TYPE, 1120 + i, &size);
		temp.Unflatten((const char*)buff);
		fChatItems.AddItem(new BMessage(temp));
	}

	// Loading default chat commands
	for (int i = 0; i < 9; i++) {
		size_t size;
		BMessage temp;
		const void* buff = res.LoadResource(B_MESSAGE_TYPE, 1140 + i, &size);
		temp.Unflatten((const char*)buff);
		ChatCommand* cmd = new ChatCommand(&temp);
		fCommands.AddItem(cmd->GetName(), cmd);
	}

	fStarted = false;
}


void
Server::Quit()
{
	for (int i = 0; i < fLoopers.CountItems(); i++)
		RemoveProtocolLooper(fLoopers.KeyAt(i));
}


void
Server::LoginAll()
{
	for (uint32 i = 0; i < fLoopers.CountItems(); i++)
		Login(fLoopers.ValueAt(i));
	fStarted = true;
}


void
Server::Login(ProtocolLooper* looper)
{
	BMessage* msg = new BMessage(IM_MESSAGE);
	msg->AddInt32("im_what", IM_SET_OWN_STATUS);
	msg->AddInt32("status", STATUS_ONLINE);

	if (looper != NULL)
		looper->PostMessage(msg);
}


filter_result
Server::Filter(BMessage* message, BHandler **target)
{
	filter_result result = B_DISPATCH_MESSAGE;

	switch (message->what) {
		case IM_MESSAGE:
			result = ImMessage(message);
			break;
		case IM_ERROR:
			ImError(message);
			break;
		case APP_ACCOUNT_DISABLED:
		{
			BString name;
			if (AppPreferences::Get()->NotifyProtocolStatus == true
					&& message->FindString("name", &name) == B_OK) {
				BBitmap* icon = new BBitmap(message);
				BString content(B_TRANSLATE("%user% has been disabled!"));
				content.ReplaceAll("%user%", name);

				_SendNotification(B_TRANSLATE("Disabled"), content, icon);
			}
			break;
		}
		case APP_ACCOUNT_FAILED:
		{
			BString name;
			if (AppPreferences::Get()->NotifyProtocolStatus == true
					&& message->FindString("name", &name) == B_OK) {
				BBitmap* icon = new BBitmap(message);
				BString content(B_TRANSLATE("%user% has been temporarily disabled."));
				content.ReplaceAll("%user%", name);

				_SendNotification(B_TRANSLATE("Connection failed"), content, icon);
			}
			break;
		}
		case APP_REPLICANT_MESSENGER:
		{
			BMessenger* messenger = new BMessenger();

			status_t ret = message->FindMessenger(
				"messenger", messenger);

			if (ret != B_OK || !messenger->IsValid()) {
				printf("err %s\n", strerror(ret));
				break;
			}
			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetReplicantMessenger(messenger);
			accountManager->ReplicantStatusNotify(accountManager->Status());
			break;
		}
		case APP_ROOM_INFO:
		{
			Conversation* chat = _EnsureConversation(message);
			if (chat != NULL) {
				ConversationInfoWindow* win = new ConversationInfoWindow(chat);
				win->Show();
			}
			break;
		}
		case APP_USER_INFO:
		{
			User* user = _EnsureUser(message);
			if (user != NULL) {
				UserInfoWindow* win = new UserInfoWindow(user);
				win->Show();
			}
			break;
		}
		case APP_REQUEST_HELP:
		{
			BString body;
			BString cmd_name = message->FindString("misc_str");
			int64 instance = message->FindInt64("instance");
			Conversation* chat = _EnsureConversation(message);

			if (chat == NULL)
				break;

			if (cmd_name.IsEmpty() == false) {
				ChatCommand* cmd = CommandById(cmd_name, instance);
				if (cmd == NULL)
					body = B_TRANSLATE("-- That command doesn't exist. Try "
						"'/help' for a list.\n");
				else {
					body = "** ";
					body << cmd->GetName() << " ― " << cmd->GetDesc() << "\n";
				}
			}
			else {
				CommandMap combinedCmds;
				combinedCmds.AddList(fCommands);
				combinedCmds.AddList(chat->GetProtocolLooper()->Commands());

				body << B_TRANSLATE("** Commands: ");
				for (int i = 0; i < combinedCmds.CountItems(); i++) {
					ChatCommand* cmd = combinedCmds.ValueAt(i);
					if (i > 0)	body << ", ";
					body << cmd->GetName();
				}
				body << "\n";
			}
			BMessage* help = new BMessage(IM_MESSAGE);
			help->AddInt32("im_what", IM_MESSAGE_RECEIVED);
			help->AddString("body", body);
			help->AddInt64("when", 0);
			chat->ImMessage(help);
			break;
		}
		default:
			// Dispatch not handled messages to main window
			break;
	}
	return result;
}


filter_result
Server::ImMessage(BMessage* msg)
{
	filter_result result = B_DISPATCH_MESSAGE;
	int32 im_what = msg->FindInt32("im_what");

	switch (im_what) {
		case IM_ROSTER:
		{
			int i = 0;
			BString id;
			while (msg->FindString("user_id", i++, &id) == B_OK)
				_EnsureContact(msg);
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_ROSTER_CONTACT_REMOVED:
		{
			Contact* contact = _EnsureContact(msg);
			ProtocolLooper* looper = _LooperFromMessage(msg);

			if (looper == NULL || contact == NULL) {
				result = B_SKIP_MESSAGE;
				break;
			}
			looper->RemoveContact(contact);
			break;
		}
		case IM_OWN_STATUS_SET:
		{
			int32 status;
			ProtocolLooper* looper = _LooperFromMessage(msg);
			if (msg->FindInt32("status", &status) != B_OK || looper == NULL)
				return B_SKIP_MESSAGE;

			// If online/offline, send a message to the protocol's buffer.
			if (status == STATUS_OFFLINE || status == STATUS_ONLINE) {
				BMessage info(IM_MESSAGE);
				info.AddInt32("im_what", IM_MESSAGE_RECEIVED);
				info.AddString("user_name", "Cardie");
				if (status == STATUS_OFFLINE)
					info.AddString("body", B_TRANSLATE("Account now offline and disconnected."));
				else
					info.AddString("body", B_TRANSLATE("Account now online!"));
				looper->GetView()->MessageReceived(&info);
			}

			Contact* contact = looper->GetOwnContact();
			if (contact != NULL) {
				// Send notification, if appropriate
				UserStatus oldStatus = contact->GetNotifyStatus();
				if ((oldStatus == STATUS_ONLINE && status == STATUS_OFFLINE)
					|| (oldStatus == STATUS_OFFLINE && status == STATUS_ONLINE))
				{
					if (status == STATUS_ONLINE)
						_ProtocolNotification(looper,
							BString(B_TRANSLATE("Connected")),
							BString(B_TRANSLATE("%user% has connected!")));
					else
						_ProtocolNotification(looper,
							BString(B_TRANSLATE("Disconnected")),
							BString(B_TRANSLATE("%user% is now offline!")));
				}

				contact->SetNotifyStatus((UserStatus)status);

				BString statusMsg;
				if (msg->FindString("message", &statusMsg) == B_OK)
					contact->SetNotifyPersonalStatus(statusMsg);
			}
			break;
		}
		case IM_OWN_NICKNAME_SET:
		{
			BString nick;
			ProtocolLooper* looper = _LooperFromMessage(msg);
			if (looper == NULL || msg->FindString("user_name", &nick) != B_OK)
				break;
			Contact* contact = looper->GetOwnContact();
			if (contact != NULL)
				contact->SetNotifyName(nick.String());
			break;
		}
		case IM_USER_NICKNAME_SET:
		{
			BString nick;
			if (msg->FindString("user_name", &nick) != B_OK)
				return B_SKIP_MESSAGE;

			User* user = _EnsureUser(msg);
			if (user == NULL)
				break;

			ChatMap conv = user->Conversations();
			for (int i = 0; i < conv.CountItems(); i++)
				conv.ValueAt(i)->ImMessage(msg);

			user->SetNotifyName(nick);
			break;
		}
		case IM_USER_STATUS_SET:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;

			User* user = _EnsureUser(msg);
			if (!user)
				break;

			user->SetNotifyStatus((UserStatus)status);
			BString statusMsg;
			if (msg->FindString("message", &statusMsg) == B_OK) {
				user->SetNotifyPersonalStatus(statusMsg);
			}
			break;
		}
		case IM_OWN_CONTACT_INFO:
		{
			BString id = msg->FindString("user_id");
			ProtocolLooper* looper = _LooperFromMessage(msg);
			if (looper == NULL || id.IsEmpty() == true) break;

			Contact* contact = looper->GetOwnContact();
			if (contact == NULL) {
				contact = new Contact(id, Looper());
				contact->SetProtocolLooper(looper);
				looper->SetOwnContact(contact);
			}

			BString name;
			if (msg->FindString("user_name", &name) == B_OK)
					contact->SetNotifyName(name);
			break;
		}
		case IM_CONTACT_INFO:
		{
			Contact* contact = _EnsureContact(msg);
			if (!contact)
				break;

			const char* name = NULL;

			if ((msg->FindString("user_name", &name) == B_OK)
				&& (strcmp(name, "") != 0))
				contact->SetNotifyName(name);

			BString status;
			if (msg->FindString("message", &status) == B_OK)
				contact->SetNotifyPersonalStatus(status);
			break;
		}
		case IM_EXTENDED_CONTACT_INFO:
		{
			Contact* contact = _EnsureContact(msg);
			if (!contact)
				break;

			if (contact->GetName().Length() > 0)
				break;

			const char* name = NULL;

			if ((msg->FindString("full_name", &name) == B_OK)
				&& (strcmp(name, "") != 0))
				contact->SetNotifyName(name);
			else if ((msg->FindString("user_name", &name) == B_OK)
				&& (strcmp(name, "") != 0))
					contact->SetNotifyName(name);
			break;
		}
		case IM_OWN_AVATAR_SET:
		{
			ProtocolLooper* looper = _LooperFromMessage(msg);
			Contact* contact = looper->GetOwnContact();
			entry_ref ref;

			if (msg->FindRef("ref", &ref) == B_OK) {
				BBitmap* bitmap = BTranslationUtils::GetBitmap(&ref);
				if (bitmap != NULL)
					contact->SetNotifyAvatarBitmap(bitmap);
			}
			break;
		}
		case IM_USER_AVATAR_SET:
		{
			User* user = _EnsureUser(msg);
			if (!user)
				break;

			entry_ref ref;

			if (msg->FindRef("ref", &ref) == B_OK) {
				BBitmap* bitmap = BTranslationUtils::GetBitmap(&ref);
				user->SetNotifyAvatarBitmap(bitmap);
			} else
				user->SetNotifyAvatarBitmap(NULL);
			break;
		}
		case IM_CREATE_CHAT:
		{
			BString user_id = msg->FindString("user_id");
			if (user_id.IsEmpty() == false) {
				User* user = ContactById(user_id, msg->FindInt64("instance"));
				user->GetProtocolLooper()->PostMessage(msg);
			}
			break;
		}
		case IM_CHAT_CREATED:
		{
			Conversation* chat = _EnsureConversation(msg);
			User* user = _EnsureUser(msg);

			if (chat != NULL && user != NULL) {
				chat->AddUser(user);
				chat->ShowView(false, true);
			}

			break;
		}
		case IM_JOIN_ROOM:
		{
			SendProtocolMessage(msg);
			break;
		}
		case IM_ROOM_PARTICIPANTS:
		{
			Conversation* chat = _EnsureConversation(msg);
			BStringList ids;
			BStringList name;

			msg->FindStrings("user_name", &name);
			if (msg->FindStrings("user_id", &ids) != B_OK)
				break;

			ProtocolLooper* protoLooper = _LooperFromMessage(msg);

			for (int i = 0; i < ids.CountStrings(); i++) {
				User* user = _EnsureUser(ids.StringAt(i), protoLooper);

				if (name.CountStrings() >= i && !name.StringAt(i).IsEmpty())
					user->SetNotifyName(name.StringAt(i));
				chat->AddUser(user);
			}
			break;
		}
		case IM_MESSAGE_RECEIVED:
			if (msg->HasString("chat_id") == false) {
				ProtocolLooper* looper = _LooperFromMessage(msg);
				if (looper != NULL)
					looper->GetView()->MessageReceived(msg);
				return B_SKIP_MESSAGE;
			}
		case IM_MESSAGE_SENT:
		case IM_ROOM_JOINED:
		case IM_ROOM_CREATED:
		case IM_ROOM_METADATA:
		case IM_ROOM_ROLECHANGED:
		case IM_ROOM_PARTICIPANT_JOINED:
		case IM_ROOM_PARTICIPANT_LEFT:
		case IM_ROOM_PARTICIPANT_BANNED:
		case IM_ROOM_PARTICIPANT_KICKED:
		{
			Conversation* chat = _EnsureConversation(msg);
			if (chat != NULL)
				chat->ImMessage(msg);
			break;
		}
		case IM_ROOM_NAME_SET:
		{
			BString name;
			Conversation* chat = _EnsureConversation(msg);
			if (msg->FindString("chat_name", &name) != B_OK || chat == NULL)
				break;
			chat->SetNotifyName(name.String());
			break;
		}
		case IM_ROOM_SUBJECT_SET:
		{
			BString subject;
			Conversation* chat = _EnsureConversation(msg);
			if (msg->FindString("subject", &subject) != B_OK || chat == NULL)
				break;
			chat->SetNotifySubject(subject.String());
			break;
		}
		case IM_SEND_MESSAGE:
		{
			// Route this message through the appropriate ProtocolLooper
			Conversation* conversation = _EnsureConversation(msg);
			if (conversation->GetProtocolLooper())
				conversation->GetProtocolLooper()->PostMessage(msg);
			break;
		}
		case IM_ROOM_INVITE_RECEIVED:
		{
			BString chat_id;
			User* user = _EnsureUser(msg);
			BString user_id = msg->FindString("user_id");
			BString user_name = user_id;
			BString chat_name = msg->FindString("chat_name");
			BString body = msg->FindString("body");
			ProtocolLooper* looper = _LooperFromMessage(msg);

			if (msg->FindString("chat_id", &chat_id) != B_OK || looper == NULL)
			{
				result = B_SKIP_MESSAGE;
				break;
			}

			if (chat_name.IsEmpty() == true)
				chat_name = chat_id;

			if (user != NULL)
				user_name = user->GetName();

			BString alertBody(B_TRANSLATE("You've been invited to %room%."));
			if (user_id.IsEmpty() == false)
				alertBody = B_TRANSLATE("%user% has invited you to %room%.");
			if (body.IsEmpty() == false)
				alertBody << "\n\n\"%body%\"";

			alertBody.ReplaceAll("%user%", user_name);
			alertBody.ReplaceAll("%room%", chat_name);
			alertBody.ReplaceAll("%body%", body);

			BMessage* accept = new BMessage(IM_MESSAGE);
			accept->AddInt32("im_what", IM_ROOM_INVITE_ACCEPT);
			accept->AddString("chat_id", chat_id);
			BMessage* reject = new BMessage(IM_MESSAGE);
			accept->AddInt32("im_what", IM_ROOM_INVITE_REFUSE);
			reject->AddString("chat_id", chat_id);

			InviteDialogue* invite = new InviteDialogue(BMessenger(looper),
										B_TRANSLATE("Invitation received"),
										alertBody.String(), accept, reject);
			invite->Go();
			break;
		}
		case IM_ROOM_PARTICIPANT_STARTED_TYPING:
		case IM_ROOM_PARTICIPANT_STOPPED_TYPING:
		{
//			User* user = _EnsureUser();
//			Conversation* chat = _EnsureConversation();
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_PROGRESS:
		{
			ProtocolLooper* looper = _LooperFromMessage(msg);
			const char* protocol = NULL;
			const char* title = NULL;
			const char* message = NULL;
			float progress = 0.0f;

			if (looper == NULL)
				return result;
			if (msg->FindString("title", &title) != B_OK)
				return result;
			if (msg->FindString("message", &message) != B_OK)
				return result;
			if (msg->FindFloat("progress", &progress) != B_OK)
				return result;

			if (!AppPreferences::Get()->NotifyProtocolStatus)
				break;

			BNotification notification(B_PROGRESS_NOTIFICATION);
			notification.SetGroup(BString(APP_NAME));
			notification.SetTitle(title);
			notification.SetIcon(looper->Protocol()->Icon());
			notification.SetContent(message);
			notification.SetProgress(progress);
			notification.Send();
			break;
		}
		case IM_NOTIFICATION: {
			_ProtocolNotification(_LooperFromMessage(msg),
				msg->FindString("title"), msg->FindString("message"),
				(notification_type)msg->GetInt32("type",
					B_INFORMATION_NOTIFICATION));
			break;
		}
		case IM_PROTOCOL_RELOAD_COMMANDS:
		{
			ProtocolLooper* looper = _LooperFromMessage(msg);
			if (looper == NULL) break;
			looper->LoadCommands();
			break;
		}
		case IM_PROTOCOL_READY:
		{
			ProtocolLooper* looper = _LooperFromMessage(msg);
			if (looper == NULL) break;

			fAccountEnabled.AddItem(looper->Protocol()->GetName(), true);

			// Ready notification
			if (AppPreferences::Get()->NotifyProtocolStatus == true)
				_ProtocolNotification(looper, BString(B_TRANSLATE("Connected")),
					BString(B_TRANSLATE("%user% has connected!")));

			// Join cached rooms
			BEntry entry;
			char fileName[B_FILE_NAME_LENGTH] = {'\0'};
			BDirectory dir(RoomsCachePath(looper->Protocol()->GetName()));

			while (dir.GetNextEntry(&entry, true) == B_OK)
				if (entry.GetName(fileName) == B_OK) {
					int32 flags;
					BFile file(&entry, B_READ_ONLY);
					if (file.InitCheck() != B_OK)
						continue;

					if (file.ReadAttr("Chat:flags", B_INT32_TYPE, 0, &flags,
							sizeof(int32)) < 0)
						continue;

					if (!(flags & ROOM_AUTOJOIN) && !(flags & ROOM_AUTOCREATE))
						continue;

					BMessage join(IM_MESSAGE);
					int32 im_what = IM_JOIN_ROOM;
					if (flags & ROOM_AUTOCREATE) {
						im_what = IM_CREATE_CHAT;
						join.AddString("user_id", fileName);
					}
					join.AddInt32("im_what", im_what);
					join.AddString("chat_id", fileName);
					looper->PostMessage(&join);
				}
			NotifyInteger(INT_ACCOUNTS_UPDATED, 0);
			break;
		}
		case IM_PROTOCOL_DISABLE:
		{
			int64 instance = 0;
			if (msg->FindInt64("instance", &instance) != B_OK)
				result = B_SKIP_MESSAGE;
			else
				RemoveProtocolLooper(instance);

			NotifyInteger(INT_ACCOUNTS_UPDATED, 0);
			break;
		}
		default:
			break;
	}

	return result;
}


void
Server::ImError(BMessage* msg)
{
	const char* error = NULL;
	const char* detail = msg->FindString("detail");
	ProtocolLooper* looper = _LooperFromMessage(msg);

	if (msg->FindString("error", &error) != B_OK || looper == NULL)
		return;

	// Format error message
	BString errMsg(looper->Protocol()->GetName());
	errMsg << " ― " << error;
	if (detail)
		errMsg << "\n\n" << detail;

	BAlert* alert = new BAlert(B_TRANSLATE("Error"), errMsg.String(),
		B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();
}


void
Server::AddProtocolLooper(bigtime_t instanceId, ChatProtocol* cayap)
{
	ProtocolLooper* looper = new ProtocolLooper(cayap, instanceId);
	fLoopers.AddItem(instanceId, looper);
	fAccounts.AddItem(cayap->GetName(), instanceId);
	fAccountEnabled.AddItem(cayap->GetName(), false);

	if (fStarted == true)
		Login(looper);
}


void
Server::RemoveProtocolLooper(bigtime_t instanceId)
{
	ProtocolLooper* looper = GetProtocolLooper(instanceId);
	if (looper == NULL)
		return;

	ChatMap chats = looper->Conversations();
	for (int i = 0; i < chats.CountItems(); i++)
		delete chats.ValueAt(i);

	UserMap users = looper->Users();
	for (int i = 0; i < users.CountItems(); i++)
		delete users.ValueAt(i);

	fLoopers.RemoveItemFor(instanceId);
	fAccounts.RemoveItemFor(looper->Protocol()->GetName());
	fAccountEnabled.AddItem(looper->Protocol()->GetName(), false);
	looper->Lock();
	looper->Quit();
}


ProtocolLooper*
Server::GetProtocolLooper(bigtime_t instanceId)
{
	bool found = false;
	return fLoopers.ValueFor(instanceId, &found);
}


AccountInstances
Server::GetAccounts()
{
	return fAccounts;
}


AccountInstances
Server::GetActiveAccounts()
{
	AccountInstances fActive;
	for (int i = 0; i < fAccounts.CountItems(); i++)
		if (fAccountEnabled.ValueFor(fAccounts.KeyAt(i)) == true)
			fActive.AddItem(fAccounts.KeyAt(i), fAccounts.ValueAt(i));
	return fActive;
}


void
Server::SendProtocolMessage(BMessage* msg)
{
	// Skip null messages
	if (!msg)
		return;

	// Check if message contains the instance field
	bigtime_t id;
	if (msg->FindInt64("instance", &id) == B_OK) {
		bool found = false;
		ProtocolLooper* looper
			= fLoopers.ValueFor(id, &found);

		if (found)
			looper->PostMessage(msg);
	}
}
 

void
Server::SendAllProtocolMessage(BMessage* msg)
{
	// Skip null messages
	if (!msg)
		return;

	// Send message to all protocols
	for (uint32 i = 0; i < fLoopers.CountItems(); i++) {
		ProtocolLooper* looper = fLoopers.ValueAt(i);
		looper->PostMessage(msg);
	}
}


RosterMap
Server::Contacts() const
{
	RosterMap contacts;

	for (int i = 0; i < fAccounts.CountItems(); i++) {
		ProtocolLooper* fruitLoop = fLoopers.ValueFor(fAccounts.ValueAt(i));
		if (fruitLoop == NULL)	continue;
		contacts.AddList(fruitLoop->Contacts());
	}

	return contacts;
}


Contact*
Server::ContactById(BString id, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	Contact* result = NULL;
	if (looper != NULL)
		result = looper->ContactById(id);
	return result;
}


void
Server::AddContact(Contact* contact, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	if (looper != NULL)
		looper->AddContact(contact);
}


UserMap
Server::Users() const
{
	UserMap users;
	for (int i = 0; i < fAccounts.CountItems(); i++) {
		ProtocolLooper* fruitLoop = fLoopers.ValueFor(fAccounts.ValueAt(i));
		if (fruitLoop == NULL)	continue;
		users.AddList(fruitLoop->Users());
	}

	return users;
}


User*
Server::UserById(BString id, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	User* result = NULL;
	if (looper != NULL)
		result = looper->UserById(id);
	return result;
}


void
Server::AddUser(User* user, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	if (looper != NULL)
		looper->AddUser(user);
}


ChatMap
Server::Conversations() const
{
	ChatMap chats;

	for (int i = 0; i < fAccounts.CountItems(); i++) {
		ProtocolLooper* fruitLoop = fLoopers.ValueFor(fAccounts.ValueAt(i));
		if (fruitLoop == NULL)	continue;
		chats.AddList(fruitLoop->Conversations());
	}

	return chats;
}


Conversation*
Server::ConversationById(BString id, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	Conversation* result = NULL;
	if (looper != NULL)
		result = looper->ConversationById(id);
	return result;
}


void
Server::AddConversation(Conversation* chat, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	if (looper != NULL)
		looper->AddConversation(chat);
}


CommandMap
Server::Commands(int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	CommandMap cmds = fCommands;
	if (looper != NULL)
		cmds.AddList(looper->Commands());
	return cmds;
}


ChatCommand*
Server::CommandById(BString id, int64 instance)
{
	ProtocolLooper* looper = fLoopers.ValueFor(instance);
	ChatCommand* result = NULL;
	if (looper != NULL)
		result = looper->CommandById(id);
	if (result == NULL)
		result = fCommands.ValueFor(id);
	return result;
}


BObjectList<BMessage>
Server::ChatPopUpItems()
{
	return fChatItems;
}


BObjectList<BMessage>
Server::UserPopUpItems()
{
	return fUserItems;
}


ProtocolLooper*
Server::_LooperFromMessage(BMessage* message)
{
	if (!message)
		return NULL;

	bigtime_t identifier;

	if (message->FindInt64("instance", &identifier) == B_OK) {
		bool found = false;

		ProtocolLooper* looper = fLoopers.ValueFor(identifier, &found);
		if (found)
			return looper;
	}

	return NULL;
}


Contact*
Server::_EnsureContact(BMessage* message)
{
	BString id = message->FindString("user_id");
	ProtocolLooper* looper = _LooperFromMessage(message);
	if (looper == NULL) return NULL;

	Contact* contact = looper->ContactById(id);

	if (contact == NULL && id.IsEmpty() == false && looper != NULL) {
		contact = new Contact(id, Looper());
		contact->SetProtocolLooper(looper);
		looper->AddContact(contact);
	}

	return contact;
}


User*
Server::_EnsureUser(BMessage* message)
{
	BString id = message->FindString("user_id");
	return _EnsureUser(id, _LooperFromMessage(message));
}


User*
Server::_EnsureUser(BString id, ProtocolLooper* protoLooper)
{
	User* user = protoLooper->UserById(id);

	if (user == NULL && id.IsEmpty() == false) {
		user = new User(id, Looper());
		user->SetProtocolLooper(protoLooper);
		protoLooper->AddUser(user);
	}

	return user;
}


Conversation*
Server::_EnsureConversation(BMessage* message)
{
	ProtocolLooper* looper;
	if (!message || (looper = _LooperFromMessage(message)) == NULL)
		return NULL;

	BString chat_id = message->GetString("chat_id", "");
	Conversation* item = NULL;

	if (chat_id.IsEmpty() == false) {
		item = looper->ConversationById(chat_id);

		if (item == NULL) {
			item = new Conversation(chat_id, Looper());
			item->SetProtocolLooper(looper);
			if (looper->GetOwnContact() != NULL)
				item->AddUser(looper->GetOwnContact());
			looper->AddConversation(item);

			BMessage meta(IM_MESSAGE);
			meta.AddInt32("im_what", IM_GET_ROOM_METADATA);
			meta.AddString("chat_id", chat_id);

			BMessage users(IM_MESSAGE);
			users.AddInt32("im_what", IM_GET_ROOM_PARTICIPANTS);
			users.AddString("chat_id", chat_id);

			looper->MessageReceived(&meta);
			looper->MessageReceived(&users);
		}
	}
	return item;
}


void
Server::_ProtocolNotification(ProtocolLooper* looper, BString title,
	BString desc, notification_type type)
{
	if (looper == NULL || title.IsEmpty() == true)	return;
	title.ReplaceAll("%user%", looper->Protocol()->GetName());
	desc.ReplaceAll("%user%", looper->Protocol()->GetName());
	_SendNotification(title, desc, looper->Protocol()->Icon(), type);
}


void
Server::_SendNotification(BString title, BString content, BBitmap* icon,
	notification_type type)
{
	BNotification notification(type);
	notification.SetGroup(BString(APP_NAME));
	notification.SetTitle(title);
	if (content.IsEmpty() == false)
		notification.SetContent(content);
	if (icon != NULL)
		notification.SetIcon(icon);
	notification.Send();
}
