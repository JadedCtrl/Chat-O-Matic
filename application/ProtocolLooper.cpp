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

#include <Bitmap.h>
#include <String.h>

#include "Account.h"
#include "AppMessages.h"
#include "ChatProtocolMessages.h"
#include "Conversation.h"
#include "ConversationAccountItem.h"
#include "ConversationView.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "TheApp.h"


ProtocolLooper::ProtocolLooper(ChatProtocol* protocol, int64 instance)
	:
	BLooper(),
	fProtocol(protocol),
	fInstance(instance),
	fListItem(NULL),
	fMySelf(NULL)
{
	Account* account = reinterpret_cast<Account*>(
		protocol->MessengerInterface());

	LoadCommands();

	BString name(protocol->FriendlySignature());
	name << " - " << account->Name();
	SetName(name.String());

	_InitChatView();

	Run();
}


ProtocolLooper::~ProtocolLooper()
{
	BMessage* msg = new BMessage(APP_ACCOUNT_DISABLED);
	BBitmap* icon = fProtocol->Icon();

	if (icon != NULL)
		icon->Archive(msg);
	msg->AddString("name", fProtocol->GetName());
	fProtocol->MessengerInterface()->SendMessage(msg);

	fProtocol->Shutdown();
	delete fProtocol;
}


void	
ProtocolLooper::MessageReceived(BMessage* msg)
{
	if (Protocol()->Process(msg) != B_OK)
		BLooper::MessageReceived(msg);
}


ConversationView*
ProtocolLooper::GetView()
{
	return fSystemChatView;
}


void
ProtocolLooper::ShowView()
{
	MainWindow* win = ((TheApp*)be_app)->GetMainWindow();
	win->SetConversation(NULL);
	win->SetConversationView(fSystemChatView);
}


ChatProtocol*
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
	if (chat != NULL)
		fChatMap.AddItem(chat->GetId(), chat);
}


void
ProtocolLooper::RemoveConversation(Conversation* chat)
{
	if (chat != NULL)
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
	if (contact != NULL)
		fRosterMap.AddItem(contact->GetId(), contact);
}


void
ProtocolLooper::RemoveContact(Contact* contact)
{
	if (contact == NULL)
		return;
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
	if (user != NULL)
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


Contact*
ProtocolLooper::GetOwnContact()
{
	return fMySelf;
}


void
ProtocolLooper::SetOwnContact(Contact* contact)
{
	if (contact != NULL) {
		fMySelf = contact;
		AddUser(contact);
	}
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
				fInstance, this);
	return fListItem;
}


void
ProtocolLooper::LoadCommands()
{
	fCommands = CommandMap();
	BObjectList<BMessage> commands = fProtocol->Commands();
	for (int i = 0; i < commands.CountItems(); i++) {
		ChatCommand* cmd = new ChatCommand(commands.ItemAt(i));
		fCommands.AddItem(cmd->GetName(), cmd);
	}
}


void
ProtocolLooper::_InitChatView()
{
	fSystemChatView = new ConversationView();

	BMessage clear(kClearText);
	fSystemChatView->MessageReceived(&clear);

	BMessage init(IM_MESSAGE);
	init.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	init.AddString("user_name", "Cardie");
	init.AddString("body", B_TRANSLATE("I'm rearing to go!"));
	fSystemChatView->MessageReceived(&init);

	fSystemChatView->ObserveString(STR_ROOM_NAME, fProtocol->GetName());
	fSystemChatView->ObserveString(STR_ROOM_SUBJECT, "System buffer");

	BBitmap* icon = fProtocol->Icon();
	if (icon != NULL)
		fSystemChatView->ObservePointer(PTR_ROOM_BITMAP, (void*)icon);
}
