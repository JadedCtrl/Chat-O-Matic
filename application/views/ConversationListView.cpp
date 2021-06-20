/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationListView.h"

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


const uint32 kOpenSelectedChat = 'CVos';
const uint32 kLeaveSelectedChat = 'CVcs';


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
			ConversationItem* item;
			int32 selIndex = CurrentSelection();

			if (selIndex >= 0
				&& (item = (ConversationItem*)ItemAt(selIndex)) != NULL
				&& item->OutlineLevel() == 1)
				item->GetConversation()->ShowView(false, true);
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

	// Don't allow selecting an AccountItem
	if (newSel >= 0 && ItemAt(newSel)->OutlineLevel() == 0) {
		Select(selection);
		return;
	}

	// Don't allow deselecting a room
	if (newSel < 0 && selection >= 0)
		Select(selection);

	uint32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32*)&buttons);

	if (!(buttons & B_SECONDARY_MOUSE_BUTTON))
		return;

	if (CurrentSelection() >= 0)
		_ConversationPopUp()->Go(ConvertToScreen(where), true, false);
	else
		_BlankPopUp()->Go(ConvertToScreen(where), true, false);
}


void
ConversationListView::SelectionChanged()
{
	MessageReceived(new BMessage(kOpenSelectedChat));
}


void
ConversationListView::AddConversation(Conversation* chat)
{
	ConversationAccountItem* superItem = _EnsureAccountItem(chat);
	ConversationItem* item = chat->GetListItem();
	if (superItem == NULL || item == NULL)
		return;

	AddUnder(item, superItem);
}


void
ConversationListView::RemoveConversation(Conversation* chat)
{
	RemoveItem(chat->GetListItem());
}


int32
ConversationListView::CountConversations()
{
	int32 count = 0;
	for (int32 i = 0; i < CountItems(); i++)
		if (ItemAt(i)->OutlineLevel() == 1)
			count++;
	return count;
}


int32
ConversationListView::ConversationIndexOf(Conversation* chat)
{
	ConversationItem* item = chat->GetListItem();
	int32 index = IndexOf(item);
	int32 chatIndex = index;

	if (item == NULL || index < 0)
		return -1;

	for (int i = 0; i < index; i++)
		if (ItemAt(i)->OutlineLevel() == 0) // If AccountItem
			chatIndex--;
	return chatIndex;
}


void
ConversationListView::SelectConversation(int32 index)
{
	for (int32 i = 0, cindex = -1; i < CountItems(); i++) {
		if (ItemAt(i)->OutlineLevel() == 1) // If ConversationItem
			cindex++;

		if (cindex == index) {
			Select(i);
			break;
		}
	}
}


BPopUpMenu*
ConversationListView::_ConversationPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("chatPopUp");
	int32 selIndex = CurrentSelection();

	ConversationItem* item;
	if ((item = (ConversationItem*)ItemAt(selIndex)) == NULL)
		return _BlankPopUp();
	Conversation* chat = item->GetConversation();
	ProtocolLooper* looper = chat->GetProtocolLooper();

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	BObjectList<BMessage> items = server->ChatPopUpItems();
	BObjectList<BMessage> protoItems = looper->Protocol()->ChatPopUpItems();
	items.AddList(&protoItems);

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


BPopUpMenu*
ConversationListView::_BlankPopUp()
{
	bool enabled = false;

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	if (server != NULL && server->GetAccounts().CountItems() > 0)
		enabled = true;

	BPopUpMenu* menu = new BPopUpMenu("blankPopUp");
	BMenuItem* newChat = new BMenuItem("New chat" B_UTF8_ELLIPSIS,
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


