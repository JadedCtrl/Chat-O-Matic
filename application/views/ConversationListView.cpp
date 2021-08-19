/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationListView.h"

#include <Catalog.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "AppMessages.h"
#include "ChatProtocolMessages.h"
#include "Conversation.h"
#include "ConversationAccountItem.h"
#include "ConversationItem.h"
#include "MainWindow.h"
#include "ProtocolLooper.h"
#include "Server.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConversationListView"


const uint32 kOpenSelectedChat = 'CVos';
const uint32 kLeaveSelectedChat = 'CVcs';


static int
compare_by_name(const BListItem* _item1, const BListItem* _item2)
{
	ConversationItem* item1 = (ConversationItem*)_item1;
	ConversationItem* item2 = (ConversationItem*)_item2;

	return strcasecmp(item1->GetConversation()->GetName().String(),
		item2->GetConversation()->GetName().String());
}


static int
compare_conversations(const BListItem* _item1, const BListItem* _item2)
{
	ConversationItem* item1 = (ConversationItem*)_item1;
	ConversationItem* item2 = (ConversationItem*)_item2;

	int32 userCount1 = item1->GetConversation()->Users().CountItems();
	int32 userCount2 = item2->GetConversation()->Users().CountItems();

	// Sort by name among chats/rooms
	if ((userCount1 <= 2 && userCount2 <= 2) 
			|| (userCount1 > 2 && userCount2 > 2))
		return compare_by_name(item1, item2);

	// One-on-one chats should sort above rooms
	if (userCount1 <=2 && userCount2 > 2)
		return -1;
	if (userCount1 > 2 && userCount2 <= 2)
		return 1;
	return 0;
}


ConversationListView::ConversationListView(const char* name)
	: BOutlineListView(name)
{
}


void
ConversationListView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kOpenSelectedChat:
		{
			int32 selIndex = CurrentSelection();
			if (selIndex < 0)
				break;

			ConversationItem* citem
				= dynamic_cast<ConversationItem*>(ItemAt(selIndex));
			ConversationAccountItem* caitem
				= dynamic_cast<ConversationAccountItem*>(ItemAt(selIndex));

			if (citem != NULL)
				citem->GetConversation()->ShowView(false, true);
			else if (caitem != NULL)
				caitem->GetLooper()->ShowView();
			break;
		}
		default:
			BListView::MessageReceived(msg);
	}
}


void
ConversationListView::MouseDown(BPoint where)
{
	int32 selection = CurrentSelection();
	BOutlineListView::MouseDown(where);
	int32 newSel = CurrentSelection();

	// Don't allow deselecting anything
	if (newSel < 0 && selection >= 0)
		Select(selection);

	uint32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32*)&buttons);

	if (!(buttons & B_SECONDARY_MOUSE_BUTTON))
		return;

	if (CurrentSelection() >= 0) {
		BPopUpMenu* menu = _ConversationPopUp();
		if (menu != NULL)
			menu->Go(ConvertToScreen(where), true, false);
	}
	else
		_BlankPopUp()->Go(ConvertToScreen(where), true, false);
}


void
ConversationListView::SelectionChanged()
{
	MessageReceived(new BMessage(kOpenSelectedChat));
}


void
ConversationListView::RemoveItemSelecting(BListItem* item)
{
	int32 selection = CurrentSelection();
	int32 itemIndex = IndexOf(item);
	RemoveItem(item);

	if (itemIndex == selection && CountItems() > selection)
		Select(selection);
	else if (itemIndex == selection && CountItems() >= 1)
		Select(0);
}


void
ConversationListView::AddConversation(Conversation* chat)
{
	ConversationAccountItem* superItem = _EnsureAccountItem(chat);
	ConversationItem* item = chat->GetListItem();
	if (superItem == NULL || item == NULL)
		return;

	AddUnder(item, superItem);
	SortItemsUnder(superItem, true, compare_conversations);
}


void
ConversationListView::RemoveConversation(Conversation* chat)
{
	RemoveItemSelecting(chat->GetListItem());
}


void
ConversationListView::AddAccount(int64 instance)
{
	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	ProtocolLooper* looper = server->GetProtocolLooper(instance);
	if (looper == NULL)
		return;
	AddItem(looper->GetListItem());
	if (CurrentSelection() < 0)
		Select(0);
}


void
ConversationListView::RemoveAccount(int64 instance)
{
	int32 selection = 0;
	for (int i = 0; i < CountItems(); i++) {
		ConversationAccountItem* item
			= dynamic_cast<ConversationAccountItem*>(ItemAt(i));
		if (item != NULL && item->GetInstance() == instance) {
			RemoveItemSelecting(item);
			break;
		}
	}
	if (CountItems() == 0)
		((TheApp*)be_app)->GetMainWindow()->SetConversation(NULL);
}


void
ConversationListView::SortConversation(Conversation* chat)
{
	ConversationAccountItem* superItem = _EnsureAccountItem(chat);
	SortItemsUnder(superItem, true, compare_conversations);
}


BPopUpMenu*
ConversationListView::_ConversationPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("chatPopUp");
	menu->SetRadioMode(false);
	int32 selIndex = CurrentSelection();

	ConversationItem* item;
	if ((item = dynamic_cast<ConversationItem*>(ItemAt(selIndex))) == NULL)
		return NULL;
	Conversation* chat = item->GetConversation();
	ProtocolLooper* looper = chat->GetProtocolLooper();

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	_AddDefaultItems(menu, chat);
	BObjectList<BMessage> items = looper->Protocol()->ChatPopUpItems();

	if (items.CountItems() > 0)
		menu->AddSeparatorItem();

	for (int i = 0; i < items.CountItems(); i++) {
		BMessage* itemMsg = items.ItemAt(i);
		BMessage* msg = new BMessage(*itemMsg);
		BMessage toSend;
		msg->FindMessage("_msg", &toSend);
		toSend.AddString("chat_id", chat->GetId());
		toSend.AddInt64("instance", looper->GetInstance());
		msg->ReplaceMessage("_msg", &toSend);

		BMenuItem* item = new BMenuItem(msg);
		if (msg->GetBool("x_to_protocol", true) == true)
			item->SetTarget(looper);
		else
			item->SetTarget(Window());
		menu->AddItem(item);
	}
	return menu;
}


#define add_flag_item(name, flag) { \
	msg = new BMessage(APP_ROOM_FLAG); \
	msg->AddString("chat_id", id); \
	msg->AddInt64("instance", instance); \
	msg->AddInt32("flag", flag); \
\
	item = new BMenuItem(name, msg); \
	item->SetTarget(Window()); \
\
	if (!(chat->DisallowedFlags() &flag)) { \
		if (chat->GetFlags() & flag) \
			item->SetMarked(true); \
		menu->AddItem(item); \
	} \
}


void
ConversationListView::_AddDefaultItems(BPopUpMenu* menu,
	Conversation* chat)
{
	if (chat == NULL || menu == NULL)
		return;
	BString id = chat->GetId();
	ProtocolLooper* looper = chat->GetProtocolLooper();
	int64 instance = looper->GetInstance();

	BMessage* infoMsg = new BMessage(APP_ROOM_INFO);
	infoMsg->AddString("chat_id", id);
	infoMsg->AddInt64("instance", instance);
	BMenuItem* joinItem = new BMenuItem(B_TRANSLATE("Room info…"), infoMsg);
	joinItem->SetTarget(Window());
	menu->AddItem(joinItem);

	menu->AddSeparatorItem();

	BMessage* msg;
	BMenuItem* item;
	add_flag_item(B_TRANSLATE("Auto-join room"), ROOM_AUTOJOIN);
	add_flag_item(B_TRANSLATE("Log messages"), ROOM_LOG_LOCALLY);
	add_flag_item(B_TRANSLATE("Notify on every message"), ROOM_NOTIFY_ALL);
	add_flag_item(B_TRANSLATE("Notify on direct-messages"), ROOM_NOTIFY_DM);

	menu->AddSeparatorItem();

	BMessage* leaveMsg = new BMessage(IM_MESSAGE);
	leaveMsg->AddInt32("im_what", IM_LEAVE_ROOM);
	leaveMsg->AddString("chat_id", id);
	leaveMsg->AddInt64("instance", instance);
	BMenuItem* leave = new BMenuItem(B_TRANSLATE("Leave room"), leaveMsg);
	leave->SetTarget(looper);
	menu->AddItem(leave);
}


BPopUpMenu*
ConversationListView::_BlankPopUp()
{
	bool enabled = false;

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	if (server != NULL && server->GetAccounts().CountItems() > 0)
		enabled = true;

	BPopUpMenu* menu = new BPopUpMenu("blankPopUp");
	BMenuItem* newChat = new BMenuItem(B_TRANSLATE("New chat" B_UTF8_ELLIPSIS),
		new BMessage(APP_NEW_CHAT), 'M', B_COMMAND_KEY);
	newChat->SetEnabled(enabled);

	menu->AddItem(newChat);
	menu->SetTargetForItems(Window());
	return menu;
}


ConversationAccountItem*
ConversationListView::_EnsureAccountItem(Conversation* chat)
{
	ProtocolLooper* looper;
	if (chat == NULL || (looper = chat->GetProtocolLooper()) == NULL)
		return NULL;

	ConversationAccountItem* item = looper->GetListItem();
	if (HasItem(item) == false)
		AddItem(item);

	return item;
}


