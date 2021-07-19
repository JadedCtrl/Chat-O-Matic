/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserListView.h"

#include <Catalog.h>
#include <PopUpMenu.h>
#include <MenuItem.h>
#include <Window.h>

#include "AppMessages.h"
#include "ChatProtocolMessages.h"
#include "Conversation.h"
#include "MainWindow.h"
#include "ProtocolLooper.h"
#include "Role.h"
#include "Server.h"
#include "TheApp.h"
#include "User.h"
#include "UserInfoWindow.h"
#include "UserItem.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "UserListView"


UserListView::UserListView(const char* name)
	: BListView(name),
	fChat(NULL)
{
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
	UserItem* item = (UserItem*)ItemAt(CurrentSelection());
	User* selected_user;
	if (item == NULL || (selected_user = item->GetUser()) == NULL)
		return _BlankPopUp();

	Role* own_role = fChat->GetRole(fChat->GetOwnContact()->GetId());

	Role* selected_role = fChat->GetRole(selected_user->GetId());

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	BObjectList<BMessage> items = server->UserPopUpItems();
	BObjectList<BMessage> protoItems = fChat->GetProtocolLooper()->Protocol()->UserPopUpItems();
	items.AddList(&protoItems);

	for (int i = 0; i < items.CountItems(); i++) {
		BMessage* itemMsg = items.ItemAt(i);
		_ProcessItem(itemMsg, menu, own_role, selected_role, selected_user->GetId());
	}
	return menu;
}


BPopUpMenu*
UserListView::_BlankPopUp()
{
	BPopUpMenu* menu = new BPopUpMenu("blankPopUp");

	BMenuItem* invite = new BMenuItem(B_TRANSLATE("Invite user"
		B_UTF8_ELLIPSIS), new BMessage(APP_SEND_INVITE), 'I', B_COMMAND_KEY);
	if (fChat == NULL)
		invite->SetEnabled(false);

	menu->AddItem(invite);
	menu->SetTargetForItems(Window());
	
	return menu;
}


void
UserListView::_ProcessItem(BMessage* itemMsg, BPopUpMenu* menu, Role* user,
						   Role* target, BString target_id)
{
	BMessage* msg = new BMessage(*itemMsg);
	bool priority = msg->GetBool("x_priority", false);
	int32 perms = msg->GetInt32("x_perms", 0);
	int32 target_perms = msg->GetInt32("x_target_perms", 0);
	int32 target_antiperms = msg->GetInt32("x_target_antiperms", 0);

	BMessage toSend;
	msg->FindMessage("_msg", &toSend);
	toSend.AddString("user_id", target_id);
	toSend.AddString("chat_id", fChat->GetId());
	toSend.AddInt64("instance", fChat->GetProtocolLooper()->GetInstance());
	msg->ReplaceMessage("_msg", &toSend);

	if ((perms == 0 || ((user != NULL) && (user->fPerms & perms)))
		&& ((target == NULL) ||
			((target_perms == 0 || (target->fPerms & target_perms))
			&& (target_antiperms == 0 || (!(target->fPerms & target_antiperms)))
			&& ((priority == false) || (user->fPriority > target->fPriority)))))
	{
		BMenuItem* item = new BMenuItem(msg);
		if (msg->GetBool("x_to_protocol", true) == true)
			item->SetTarget(fChat->GetProtocolLooper());
		else
			item->SetTarget(Window());

		menu->AddItem(item);
	}
}


