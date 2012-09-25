/*
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Dario Casalinuovo, dario.casalinuovo@gmail.com
 */

#include "CayaConstants.h"
#include "SearchBarTextControl.h"

#include <Font.h>
#include <String.h>
#include <Window.h>

#include <stdio.h>


SearchBarTextControl::SearchBarTextControl(BMessage* message)
	:
	BTextControl("searchBox", NULL, NULL, message)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rgb_color color = tint_color(ViewColor(), B_DARKEN_3_TINT);
	TextView()->SetFontAndColor(NULL, B_FONT_ALL, &color);
	TextView()->MakeSelectable(false);
	SetModificationMessage(message);
}


void
SearchBarTextControl::KeyDown(const char* bytes, int32 numBytes)
{
}
