/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *
 * Contributors:
 *		Dario Casalinuovo
 */

#include <Application.h>
#include <Debug.h>
#include <Entry.h>
#include <Notification.h>
#include <Path.h>
#include <TranslationUtils.h>

#include "Account.h"
#include "AccountManager.h"
#include "ProtocolLooper.h"
#include "CayaMessages.h"
#include "CayaProtocol.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "ImageCache.h"
#include "ProtocolManager.h"
#include "RosterItem.h"
#include "Server.h"


Server::Server()
	:
	BMessageFilter(B_ANY_DELIVERY, B_ANY_SOURCE)
{
}


void
Server::Quit()
{
	Contact* contact = NULL;
	Conversation* conversation = NULL;

	while (contact = fRosterMap.ValueAt(0)) {
		contact->DeletePopUp();
		fRosterMap.RemoveItemAt(0);
	}

	while (conversation = fChatMap.ValueAt(0)) {
		fChatMap.RemoveItemAt(0);
		delete conversation;
	}
}


void
Server::LoginAll()
{
	for (uint32 i = 0; i < fLoopers.CountItems(); i++) {
		ProtocolLooper* looper = fLoopers.ValueAt(i);

		BMessage* msg = new BMessage(IM_MESSAGE);
		msg->AddInt32("im_what", IM_SET_OWN_STATUS);
		msg->AddInt32("status", CAYA_ONLINE);
		looper->PostMessage(msg);
	}
}


filter_result
Server::Filter(BMessage* message, BHandler **target)
{
	filter_result result = B_DISPATCH_MESSAGE;

	switch (message->what) {
		case CAYA_CLOSE_CHAT_WINDOW:
		{
			BString id = message->FindString("chat_id");
			if (id.Length() > 0) {
				bool found = false;
				Conversation* item = fChatMap.ValueFor(id, &found);
			}
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_MESSAGE:
			result = ImMessage(message);
			break;

		case CAYA_REPLICANT_MESSENGER:
		{
			BMessenger* messenger = new BMessenger();

			status_t ret = message->FindMessenger(
				"messenger", messenger);

			if (ret != B_OK || !messenger->IsValid()) {
				message->PrintToStream();
				printf("err %s\n", strerror(ret));
				break;
			}
			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetReplicantMessenger(messenger);
			accountManager->ReplicantStatusNotify(accountManager->Status());
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
		case IM_CONTACT_LIST:
		{
			int i = 0;
			BString id;
			while (msg->FindString("user_id", i++, &id) == B_OK)
				_EnsureContact(msg);
			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_OWN_STATUS_SET:
		{
			int32 status;
			const char* protocol;
			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;
			if (msg->FindString("protocol", &protocol) != B_OK)
				return B_SKIP_MESSAGE;

			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetStatus((CayaStatus)status);

			break;
		}
		case IM_STATUS_SET:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return B_SKIP_MESSAGE;

			Contact* contact = _EnsureContact(msg);
			if (!contact)
				break;

			contact->SetNotifyStatus((CayaStatus)status);
			BString statusMsg;
			if (msg->FindString("message", &statusMsg) == B_OK) {
				contact->SetNotifyPersonalStatus(statusMsg);
//				contact->GetView()->UpdatePersonalMessage();
			}
			break;
		}
		case IM_OWN_CONTACT_INFO:
		{
			Contact* contact = _EnsureContact(msg);
			if (contact != NULL) {
				fMySelf = contact->GetId();
			}
			break;
		}
		case IM_CONTACT_INFO:
		{
			Contact* contact = _EnsureContact(msg);
			if (!contact)
				break;

			const char* name = NULL;

			if ((msg->FindString("name", &name) == B_OK)
				&& (strcmp(name, "") != 0))
				contact->SetNotifyName(name);

			BString status;
			if (msg->FindString("message", &status) == B_OK) {
				contact->SetNotifyPersonalStatus(status);
//				contact->GetChatWindow()->UpdatePersonalMessage();
			}
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

			if ((msg->FindString("full name", &name) == B_OK)
				&& (strcmp(name, "") != 0))
				contact->SetNotifyName(name);
			break;
		}
		case IM_AVATAR_SET:
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
				User* user = ContactById(user_id);
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
		case IM_SEND_MESSAGE:
		{
			// Route this message through the appropriate ProtocolLooper
			Conversation* conversation = _EnsureConversation(msg);
			if (conversation->GetProtocolLooper())
				conversation->GetProtocolLooper()->PostMessage(msg);
			break;
		}
		case IM_MESSAGE_SENT:
		case IM_MESSAGE_RECEIVED:
		{
			Conversation* item = _EnsureConversation(msg);
			item->ImMessage(msg);

			break;
		}
		case IM_CONTACT_STARTED_TYPING:
		case IM_CONTACT_STOPPED_TYPING:
		{
			BString id = msg->FindString("chat_id");
			Conversation* item = _EnsureConversation(msg);
			item->ImMessage(msg);

			result = B_SKIP_MESSAGE;
			break;
		}
		case IM_PROGRESS:
		{
			const char* protocol = NULL;
			const char* title = NULL;
			const char* message = NULL;
			float progress = 0.0f;

			if (msg->FindString("protocol", &protocol) != B_OK)
				return result;
			if (msg->FindString("title", &title) != B_OK)
				return result;
			if (msg->FindString("message", &message) != B_OK)
				return result;
			if (msg->FindFloat("progress", &progress) != B_OK)
				return result;

			if (!CayaPreferences::Item()->NotifyProtocolStatus)
				break;

			CayaProtocolAddOn* addOn
				= ProtocolManager::Get()->ProtocolAddOn(protocol);

			BNotification notification(B_PROGRESS_NOTIFICATION);
			notification.SetGroup(BString("Caya"));
			notification.SetTitle(title);
			notification.SetIcon(addOn->ProtoIcon());
			notification.SetContent(message);
			notification.SetProgress(progress);
			notification.Send();

			break;
		}
		case IM_NOTIFICATION:
		{
			int32 type = (int32)B_INFORMATION_NOTIFICATION;
			const char* protocol = NULL;
			const char* title = NULL;
			const char* message = NULL;

			if (msg->FindString("protocol", &protocol) != B_OK)
				return result;
			if (msg->FindInt32("type", &type) != B_OK)
				return result;
			if (msg->FindString("title", &title) != B_OK)
				return result;
			if (msg->FindString("message", &message) != B_OK)
				return result;

			if (!CayaPreferences::Item()->NotifyProtocolStatus)
				break;

			CayaProtocolAddOn* addOn
				= ProtocolManager::Get()->ProtocolAddOn(protocol);

			BNotification notification((notification_type)type);
			notification.SetGroup(BString("Caya"));
			notification.SetTitle(title);
			notification.SetIcon(addOn->ProtoIcon());
			notification.SetContent(message);
			notification.Send();

			break;
		}

		default:
			break;
	}

	return result;
}


void
Server::AddProtocolLooper(bigtime_t instanceId, CayaProtocol* cayap)
{
	ProtocolLooper* looper = new ProtocolLooper(cayap);
	fLoopers.AddItem(instanceId, looper);
}


void
Server::RemoveProtocolLooper(bigtime_t instanceId)
{
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
	return fRosterMap;
}


Contact*
Server::ContactById(BString id)
{
	bool found = false;
	return fRosterMap.ValueFor(id, &found);
}


void
Server::AddContact(Contact* contact)
{
	fRosterMap.AddItem(contact->GetId(), contact);
}


UserMap
Server::Users() const
{
	UserMap users = fUserMap;

	for (int i = 0; i < fRosterMap.CountItems(); i++) {
		User* user = (User*)fRosterMap.ValueAt(i);
		users.AddItem(user->GetId(), user);
	}

	return users;
}


User*
Server::UserById(BString id)
{
	bool found = false;
	User* user = ContactById(id);
	if (user == NULL)
		user = fUserMap.ValueFor(id, &found);

	return user;
}


void
Server::AddUser(User* user)
{
	fUserMap.AddItem(user->GetId(), user);
}


ChatMap
Server::Conversations() const
{
	return fChatMap;
}


Conversation*
Server::ConversationById(BString id)
{
	bool found = false;
	return fChatMap.ValueFor(id, &found);
}


void
Server::AddConversation(Conversation* chat)
{
	fChatMap.AddItem(chat->GetId(), chat);
}


BString
Server::GetOwnContact()
{
	return fMySelf;
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
	Contact* contact = ContactById(id);

	if (contact == NULL && id.IsEmpty() == false) {
		contact = new Contact(id, Looper());
		contact->SetProtocolLooper(_LooperFromMessage(message));
		AddContact(contact);
	}

	return contact;
}


User*
Server::_EnsureUser(BMessage* message)
{
	BString id = message->FindString("user_id");
	User* user = UserById(id);

	if (user == NULL && id.IsEmpty() == false) {
		user = new User(id, Looper());
		user->SetProtocolLooper(_LooperFromMessage(message));
		AddUser(user);
	}

	return user;
}


Conversation*
Server::_EnsureConversation(BMessage* message)
{
	if (!message)
		return NULL;

	BString chat_id = message->FindString("chat_id");
	Conversation* item = NULL;

	if (chat_id.IsEmpty() == false) {
		bool found = false;
		item = fChatMap.ValueFor(chat_id, &found);

		if (!found) {
			item = new Conversation(chat_id, Looper());
			item->SetProtocolLooper(_LooperFromMessage(message));
			item->AddUser(ContactById(fMySelf));
			fChatMap.AddItem(chat_id, item);
		}
	}
	return item;
}


