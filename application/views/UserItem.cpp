/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserItem.h"

#include <InterfaceDefs.h>
#include <View.h>

#include "CayaConstants.h"
#include "NotifyMessage.h"
#include "User.h"


UserItem::UserItem(const char* name, User* user, int32 status)
	:
	BStringItem(name),
	fUser(user),
	fTextColor(ui_color(B_LIST_ITEM_TEXT_COLOR))
{
	_UpdateColor(status);
}


void
UserItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	rgb_color highColor = owner->HighColor();
	owner->SetHighColor(fTextColor);

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
			_UpdateColor(value);
			break;
		}
	}
}


User*
UserItem::GetUser()
{
	return fUser;
}


void
UserItem::_UpdateColor(int32 status)
{
	switch (status)
	{
		case CAYA_AWAY:
			fTextColor = _TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 1);
			break;
		case CAYA_INVISIBLE:
		case CAYA_DO_NOT_DISTURB:
			fTextColor = _TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 2);
			break;
		case CAYA_OFFLINE:
			fTextColor = _TintColor(ui_color(B_LIST_ITEM_TEXT_COLOR), 3);
			break;
		default:
			fTextColor = ui_color(B_LIST_ITEM_TEXT_COLOR);
	}
}


rgb_color
UserItem::_TintColor(rgb_color color, int severity)
{
	bool dark = false;
	if (color.Brightness() < 127)
		dark = true;

	switch (severity)
	{
		case 0:
			return color;
		case 1:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_1_TINT + 0.2f);
			else
				return tint_color(color, B_DARKEN_1_TINT);
		case 2:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_1_TINT);
			else
				return tint_color(color, B_DARKEN_2_TINT);
		case 3:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_2_TINT + 0.1f);
			else
				return tint_color(color, B_DARKEN_3_TINT);
	}
}


