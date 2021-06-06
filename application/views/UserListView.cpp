/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserListView.h"

#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "CayaMessages.h"
#include "Conversation.h"
#include "Role.h"
#include "User.h"
#include "UserInfoWindow.h"
#include "UserItem.h"


const uint32 kUserInfo = 'ULui';
const uint32 kDeafenUser = 'UMdu';
const uint32 kMuteUser = 'UMmu';
const uint32 kKickUser = 'UMku';
const uint32 kBanUser = 'UMbu';


UserListView::UserListView(const char* name)
	: BListView(name),
	fChat(NULL)
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
	menu->AddItem(new BMenuItem("User info" B_UTF8_ELLIPSIS,
		new BMessage(kUserInfo)));
	menu->SetTargetForItems(this);

	// Now for the moderation items
	Role* role = fChat->GetRole(fChat->OwnUserId());
	int32 perms = role->fPerms;
	UserItem* item = (UserItem*)ItemAt(CurrentSelection());
	User* selected_user;

	if (item == NULL || (selected_user = item->GetUser()) == NULL)
		return menu;

	int32 selected_priority = fChat->GetRole(selected_user->GetId())->fPriority;
	if (selected_priority > role->fPriority)
		return menu;

	if ((perms & PERM_DEAFEN) || (perms & PERM_MUTE) || (perms & PERM_KICK)
		|| (perms & PERM_BAN))
		menu->AddSeparatorItem();

	if (perms & PERM_DEAFEN)
		menu->AddItem(new BMenuItem("Deafen user", new BMessage(kDeafenUser)));
	if (perms & PERM_MUTE)
		menu->AddItem(new BMenuItem("Mute user", new BMessage(kMuteUser)));
	if (perms & PERM_KICK)
		menu->AddItem(new BMenuItem("Kick user", new BMessage(kKickUser)));
	if (perms & PERM_BAN)
		menu->AddItem(new BMenuItem("Ban user", new BMessage(kBanUser)));


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


