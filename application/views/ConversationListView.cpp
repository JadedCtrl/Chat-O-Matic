/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationListView.h"

#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "CayaMessages.h"
#include "CayaProtocolMessages.h"
#include "Conversation.h"
#include "ConversationItem.h"
#include "ProtocolLooper.h"


const uint32 kOpenSelectedChat = 'CVos';
const uint32 kLeaveSelectedChat = 'CVcs';


ConversationListView::ConversationListView(const char* name)
	: BListView(name)
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

			if ((item = (ConversationItem*)ItemAt(selIndex)) != NULL)
				item->GetConversation()->ShowView(false, true);
			break;
		}

		case kLeaveSelectedChat:
		{
			ConversationItem* item;
			int32 selIndex = CurrentSelection();

			if ((item = (ConversationItem*)ItemAt(selIndex)) == NULL)
				break;

			BMessage leave(IM_MESSAGE);
			leave.AddInt32("im_what", IM_LEAVE_ROOM);
			leave.AddString("chat_id", item->GetConversation()->GetId());
			item->GetConversation()->GetProtocolLooper()->MessageReceived(&leave);
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

	BListView::MouseDown(where);

	// Don't allow deselcting a room
	if (CurrentSelection() < 0 && selection >= 0)
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


BPopUpMenu*
ConversationListView::_ConversationPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("chatPopUp");
	menu->AddItem(new BMenuItem("Open chat" B_UTF8_ELLIPSIS,
		new BMessage(kOpenSelectedChat)));
	menu->AddItem(new BMenuItem("Leave chat", new BMessage(kLeaveSelectedChat)));
	menu->SetTargetForItems(this);

	return menu;

}


BPopUpMenu*
ConversationListView::_BlankPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("blankPopUp");
	menu->AddItem(new BMenuItem("New chat" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_NEW_CHAT), 'M', B_COMMAND_KEY));
	menu->SetTargetForItems(Window());
	
	return menu;
}


