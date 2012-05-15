/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "CayaConstants.h"
#include "NicknameTextControl.h"

#include <Font.h>


NicknameTextControl::NicknameTextControl(const char* name, BMessage* message)
	: BTextView(name, B_WILL_DRAW)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}

/*
void
NicknameTextControl::Draw(BRect updateRect)
{
//	BRect rect(Bounds());

//	FillRect(rect);
}
*/
