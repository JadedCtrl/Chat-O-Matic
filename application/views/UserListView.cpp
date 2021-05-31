/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserListView.h"

#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "CayaMessages.h"
#include "User.h"
#include "UserInfoWindow.h"
#include "UserItem.h"


const uint32 kUserInfo = 'ULui';
//const uint32 kLeaveSelectedChat = 'CVcs';


UserListView::UserListView(const char* name)
	: BListView(name)
{
}


void
UserListView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kUserInfo:
		{
			UserItem* item = (UserItem*)ItemAt(CurrentSelection());
			if (item == NULL)
				return;

			UserInfoWindow* win = new UserInfoWindow(item->GetUser());
			win->Show();
			break;
		}

		default:
			BListView::MessageReceived(msg);
	}
}


void
UserListView::MouseDown(BPoint where)
{
	BListView::MouseDown(where);

	uint32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32*)&buttons);

	if (!(buttons & B_SECONDARY_MOUSE_BUTTON))
		return;

	if (CurrentSelection() >= 0)
		_UserPopUp()->Go(ConvertToScreen(where), true, false);
	else
		_BlankPopUp()->Go(ConvertToScreen(where), true, false);
}


BPopUpMenu*
UserListView::_UserPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("userPopUp");
	menu->AddItem(new BMenuItem("User info…" B_UTF8_ELLIPSIS,
		new BMessage(kUserInfo)));
	menu->SetTargetForItems(this);

	return menu;

}


BPopUpMenu*
UserListView::_BlankPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("blankPopUp");

	BMenuItem* invite = new BMenuItem("Invite user…" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_SEND_INVITE), 'I', B_COMMAND_KEY);
	invite->SetEnabled(false);

	menu->AddItem(invite);
	menu->SetTargetForItems(Window());
	
	return menu;
}


