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

const char* kSearchText = "Search...";


SearchBarTextControl::SearchBarTextControl(BMessage* message)
	:
	BTextControl("searchBox", NULL, NULL, message)
{
	SetEventMask(B_POINTER_EVENTS);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	rgb_color color = tint_color(ViewColor(), B_DARKEN_1_TINT);
	SetText(kSearchText);
	TextView()->SetName("SearchBoxTextView");
	TextView()->SetFontAndColor(NULL, B_FONT_ALL, &color);
	TextView()->MakeSelectable(false);
}


void
SearchBarTextControl::MouseDown(BPoint position)
{
	BString text(Text());
	BPoint currentPosition;
	uint32 buttons;
	GetMouse(&currentPosition, &buttons);
	BView* view = Window()->FindView(currentPosition);

	if (view != NULL && (view->Name() == TextView()->Name()
			|| view->Name() == Name())) {
		BTextControl::MouseDown(position);
		SetText("");
	}

	if (TextView()->IsFocus()) {
		SetText("");
	} else if (!TextView()->IsFocus()) {
		SetText(kSearchText);
	}
}


void
SearchBarTextControl::KeyDown(const char* bytes, int32 numBytes)
{
	if (TextView()->IsFocus()) {
		if (Text() == kSearchText)
			SetText("");

		BTextControl::KeyDown(bytes, numBytes);
		BString text(Text());
		if (TextView()->IsFocus() && text.Length() > 0) {
			BMessage* msg = ModificationMessage();
			msg->AddPointer("source", this);
			Window()->MessageReceived(msg);
		}
	}
}
