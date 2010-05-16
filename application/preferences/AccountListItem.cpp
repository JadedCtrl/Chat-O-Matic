/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <View.h>

#include "AccountListItem.h"
#include "ProtocolSettings.h"


AccountListItem::AccountListItem(ProtocolSettings* settings, const char* account)
	: BStringItem(account),
	fSettings(settings),
	fAccount(account),
	fBaselineOffset(0)
{
}


AccountListItem::~AccountListItem()
{
}


ProtocolSettings*
AccountListItem::Settings() const
{
	return fSettings;
}


const char*
AccountListItem::Account() const
{
	return fAccount.String();
}


void
AccountListItem::SetAccount(const char* name)
{
	fAccount = name;
	SetText(fAccount);
}


void
AccountListItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	rgb_color highColor = owner->HighColor();
	rgb_color lowColor = owner->LowColor();

	// Draw selection
    if (IsSelected()) {
		rgb_color highlightColor = ui_color(B_CONTROL_HIGHLIGHT_COLOR);

		owner->SetLowColor(highlightColor);
		owner->SetHighColor(highlightColor);
		owner->FillRect(frame);
	}

	// Draw account name
	rgb_color textColor = ui_color(B_CONTROL_TEXT_COLOR);
    owner->MovePenTo(frame.left, frame.top + fBaselineOffset);
	if (!IsEnabled())
		owner->SetHighColor(tint_color(textColor, B_LIGHTEN_2_TINT));
	else
	    owner->SetHighColor(textColor);
	owner->DrawString(Text());

	owner->SetHighColor(highColor);
	owner->SetLowColor(lowColor);
}


void
AccountListItem::Update(BView* owner, const BFont* font)
{
	if (Text())
		SetWidth(font->StringWidth(Text()));

	font_height height;
	font->GetHeight(&height);

	fBaselineOffset = 2 + ceilf(height.ascent + height.leading / 2);

	SetHeight(ceilf(height.ascent) + ceilf(height.descent)
		+ ceilf(height.leading) + 4);
}
