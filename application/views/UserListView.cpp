/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserListView.h"

#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "CayaMessages.h"
#include "CayaProtocolMessages.h"
#include "Conversation.h"
#include "ProtocolLooper.h"
#include "Role.h"
#include "User.h"
#include "UserInfoWindow.h"
#include "UserItem.h"



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
		case kKickUser:
			_ModerationAction(IM_ROOM_KICK_PARTICIPANT);
			break;
		case kBanUser:
			_ModerationAction(IM_ROOM_BAN_PARTICIPANT);
			break;
		case kMuteUser:
			_ModerationAction(IM_ROOM_MUTE_PARTICIPANT);
			break;
		case kUnmuteUser:
			_ModerationAction(IM_ROOM_UNMUTE_PARTICIPANT);
			break;
		case kDeafenUser:
			_ModerationAction(IM_ROOM_DEAFEN_PARTICIPANT);
			break;
		case kUndeafenUser:
			_ModerationAction(IM_ROOM_UNDEAFEN_PARTICIPANT);
			break;
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
	Role* role = fChat->GetRole(fChat->GetOwnId());
	if (role == NULL)	return menu;
	int32 perms = role->fPerms;
	UserItem* item = (UserItem*)ItemAt(CurrentSelection());
	User* selected_user;

	if (item == NULL || (selected_user = item->GetUser()) == NULL)
		return menu;

	Role* selected_role = fChat->GetRole(selected_user->GetId());
	if (selected_role == NULL)	return menu;
	int32 selected_priority = selected_role->fPriority;
	int32 selected_perms = selected_role->fPerms;
	if (selected_priority > role->fPriority)
		return menu;

	if ((perms & PERM_DEAFEN) || (perms & PERM_MUTE) || (perms & PERM_KICK)
		|| (perms & PERM_BAN))
		menu->AddSeparatorItem();

	if ((perms & PERM_DEAFEN) && (selected_perms & PERM_READ))
		menu->AddItem(new BMenuItem("Deafen user", new BMessage(kDeafenUser)));
	if ((perms & PERM_DEAFEN) && !(selected_perms & PERM_READ))
		menu->AddItem(new BMenuItem("Undeafen user", new BMessage(kUndeafenUser)));

	if ((perms & PERM_MUTE) && (selected_perms & PERM_WRITE))
		menu->AddItem(new BMenuItem("Mute user", new BMessage(kMuteUser)));
	if ((perms & PERM_MUTE) && !(selected_perms & PERM_WRITE))
		menu->AddItem(new BMenuItem("Unmute user", new BMessage(kUnmuteUser)));

	if (perms & PERM_KICK)
		menu->AddItem(new BMenuItem("Kick user", new BMessage(kKickUser)));
	if (perms & PERM_BAN)
		menu->AddItem(new BMenuItem("Ban user", new BMessage(kBanUser)));

	menu->SetTargetForItems(this);
	return menu;
}


BPopUpMenu*
UserListView::_BlankPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("blankPopUp");

	BMenuItem* invite = new BMenuItem("Invite user…" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_SEND_INVITE), 'I', B_COMMAND_KEY);

	menu->AddItem(invite);
	menu->SetTargetForItems(Window());
	
	return menu;
}


void
UserListView::_ModerationAction(int32 im_what)
{
	Role* role = fChat->GetRole(fChat->GetOwnId());
	int32 perms = role->fPerms;
	UserItem* item = (UserItem*)ItemAt(CurrentSelection());
	if (item == NULL)
		return;

	BMessage modMsg(IM_MESSAGE);
	modMsg.AddInt32("im_what", im_what);
	modMsg.AddString("user_id", item->GetUser()->GetId());
	modMsg.AddString("chat_id", fChat->GetId());
	fChat->GetProtocolLooper()->PostMessage(&modMsg);
}


