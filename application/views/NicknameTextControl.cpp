/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "NicknameTextControl.h"


NicknameTextControl::NicknameTextControl(const char* name, BMessage* message)
	: BTextControl(name, NULL, NULL, message)
{
}


void
NicknameTextControl::Draw(BRect updateRect)
{
	BRect rect(Bounds());

	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	FillRect(rect);
}
