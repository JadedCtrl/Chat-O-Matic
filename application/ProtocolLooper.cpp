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
 */

#include "ProtocolLooper.h"

#include <String.h>

#include "Account.h"
#include "Conversation.h"
#include "ConversationAccountItem.h"


ProtocolLooper::ProtocolLooper(CayaProtocol* protocol, int64 instance)
	:
	BLooper(),
	fProtocol(protocol),
	fInstance(instance),
	fListItem(NULL)
{
	Account* account = reinterpret_cast<Account*>(
		protocol->MessengerInterface());

	BString name(protocol->FriendlySignature());
	name << " - " << account->Name();

	SetName(name.String());
	Run();
}


ProtocolLooper::~ProtocolLooper()
{
	fProtocol->Shutdown();
	delete fProtocol;
}


void	
ProtocolLooper::MessageReceived(BMessage* msg)
{
	if (Protocol()->Process(msg) != B_OK)
		BLooper::MessageReceived(msg);
}


CayaProtocol*
ProtocolLooper::Protocol()
{
	return fProtocol;
}


ChatMap
ProtocolLooper::Conversations() const
{
	return fChatMap;
}


Conversation*
ProtocolLooper::ConversationById(BString id)
{
	bool found = false;
	return fChatMap.ValueFor(id, &found);
}


void
ProtocolLooper::AddConversation(Conversation* chat)
{
	fChatMap.AddItem(chat->GetId(), chat);
}


void
ProtocolLooper::RemoveConversation(Conversation* chat)
{
	fChatMap.RemoveItemFor(chat->GetId());
}


RosterMap
ProtocolLooper::Contacts() const
{
	return fRosterMap;
}


Contact*
ProtocolLooper::ContactById(BString id)
{
	bool found = false;
	return fRosterMap.ValueFor(id, &found);
}


void
ProtocolLooper::AddContact(Contact* contact)
{
	fRosterMap.AddItem(contact->GetId(), contact);
}


void
ProtocolLooper::RemoveContact(Contact* contact)
{
	fRosterMap.RemoveItemFor(contact->GetId());
	fUserMap.AddItem(contact->GetId(), (User*)contact);
}


UserMap
ProtocolLooper::Users() const
{
	UserMap users = fUserMap;

	for (int i = 0; i < fRosterMap.CountItems(); i++) {
		User* user = (User*)fRosterMap.ValueAt(i);
		users.AddItem(user->GetId(), user);
	}

	return users;
}


User*
ProtocolLooper::UserById(BString id)
{
	bool found = false;
	User* user = ContactById(id);
	if (user == NULL)
		user = fUserMap.ValueFor(id, &found);

	return user;
}


void
ProtocolLooper::AddUser(User* user)
{
	fUserMap.AddItem(user->GetId(), user);
}


CommandMap
ProtocolLooper::Commands() const
{
	return fCommands;
}


ChatCommand*
ProtocolLooper::CommandById(BString id)
{
	return fCommands.ValueFor(id);
}


void
ProtocolLooper::AddCommand(ChatCommand* cmd)
{
	fCommands.AddItem(cmd->GetName(), cmd);
}


BObjectList<BMessage>
ProtocolLooper::UserPopUpItems() const
{
	return fUserItems;
}


void
ProtocolLooper::AddUserPopUpItem(BMessage* archived)
{
	fUserItems.AddItem(archived);
}


BObjectList<BMessage>
ProtocolLooper::ChatPopUpItems() const
{
	return fChatItems;
}


void
ProtocolLooper::AddChatPopUpItem(BMessage* archived)
{
	fChatItems.AddItem(archived);
}


BObjectList<BMessage>
ProtocolLooper::MenuBarItems() const
{
	return fMenuItems;
}


void
ProtocolLooper::AddMenuBarItem(BMessage* archived)
{
	fMenuItems.AddItem(archived);
}


BString
ProtocolLooper::GetOwnId()
{
	return fMySelf;
}


void
ProtocolLooper::SetOwnId(BString user_id)
{
	fMySelf = user_id;
}


int64
ProtocolLooper::GetInstance()
{
	return fInstance;
}


ConversationAccountItem*
ProtocolLooper::GetListItem()
{
	if (fListItem == NULL)
		fListItem = new ConversationAccountItem(fProtocol->GetName(),
												fInstance);
	return fListItem;
}


