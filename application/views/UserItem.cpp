/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserItem.h"

#include <InterfaceDefs.h>
#include <View.h>

#include "AppConstants.h"
#include "NotifyMessage.h"
#include "User.h"
#include "Utils.h"


UserItem::UserItem(const char* name, User* user, int32 status)
	:
	BStringItem(name),
	fUser(user),
	fStatus(status)
{
}


void
UserItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	rgb_color highColor = owner->HighColor();
	owner->SetHighColor(_GetTextColor(highColor));

	BStringItem::DrawItem(owner, frame, complete);

	owner->SetHighColor(highColor);
}


void 
UserItem::ObserveString(int32 what, BString str)
{
	switch (what) {
		case STR_CONTACT_NAME:
			SetText(str);
			break;
	}
}


void
UserItem::ObserveInteger(int32 what, int32 value)
{
	switch (what) {
		case INT_CONTACT_STATUS:
		{
			fStatus = value;
			break;
		}
	}
}


User*
UserItem::GetUser()
{
	return fUser;
}


rgb_color
UserItem::_GetTextColor(rgb_color highColor)
{
	switch (fStatus)
	{
		case STATUS_AWAY:
			return TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 1);
		case STATUS_INVISIBLE:
		case STATUS_DO_NOT_DISTURB:
			return TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 2);
		case STATUS_OFFLINE:
			return TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 3);
	}
	return highColor;
}


