/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <ListView.h>

#include <libinterface/BitmapUtils.h>

#include "AppResources.h"
#include "Contact.h"
#include "NotifyMessage.h"
#include "RosterItem.h"
#include "Utils.h"


RosterItem::RosterItem(const char*  name, Contact* contact)
	: BStringItem(name),
	fBitmap(contact->AvatarBitmap()),
	fStatus(STATUS_OFFLINE),
	contactLinker(contact),
	fVisible(true)
{
	rgb_color highlightColor = ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	rgb_color darkenHighlightColor = tint_color(highlightColor, B_DARKEN_1_TINT);

	fGradient.AddColor(highlightColor, 0);
	fGradient.AddColor(darkenHighlightColor, 255);	
}


RosterItem::~RosterItem()
{
	delete fBitmap;
}


void
RosterItem::SetVisible(bool visible)
{
	fVisible = visible;
}


void	
RosterItem::SetBitmap(BBitmap* bitmap)
{
	fBitmap = bitmap;
}


void 
RosterItem::ObserveString(int32 what, BString str)
{
	switch (what) {
		case STR_CONTACT_NAME:
			SetText(str);
			break;
		case STR_PERSONAL_STATUS:
			SetPersonalStatus(str);
			break;
	}
}


void 
RosterItem::ObservePointer(int32 what, void* ptr)
{
	switch (what) {
		case PTR_AVATAR_BITMAP:
			SetBitmap((BBitmap*)ptr);
			break;
	}
}


void 
RosterItem::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_CONTACT_STATUS:
			SetStatus((UserStatus)val);
			break;
	}
}


void
RosterItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	if (Text() == NULL)
	       return;

	rgb_color highlightColor = ui_color(B_CONTROL_HIGHLIGHT_COLOR);
	rgb_color highColor = owner->HighColor();
	rgb_color lowColor = owner->LowColor();
	float h = frame.Height();

	// Draw selection
	if (IsSelected()) {
		fGradient.SetStart(frame.LeftTop());
		fGradient.SetEnd(frame.LeftBottom());
		owner->SetLowColor(highlightColor);
		owner->FillRect(frame, fGradient);
	} else if (complete) {
		owner->SetHighColor(lowColor);
		owner->FillRect(frame);
	}

	// Draw contact status
	switch (fStatus) {
		case STATUS_ONLINE:
			owner->SetHighColor(APP_GREEN_COLOR);
			break;
		case STATUS_CUSTOM_STATUS:
		case STATUS_AWAY:
			owner->SetHighColor(APP_ORANGE_COLOR);
			break;
		case STATUS_DO_NOT_DISTURB:
			owner->SetHighColor(APP_RED_COLOR);
			break;
		case STATUS_INVISIBLE:
		case STATUS_OFFLINE:
			break;
		default:
	   		break;
	}

	owner->FillRect(BRect(frame.left,
		frame.top,
			frame.left + 5, frame.top + h - 1
			));


	// Draw avatar icon
	if (fBitmap != NULL) {
		BRect rect(frame.left + 6, frame.top,
			frame.left + 42, frame.top + h);
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		owner->DrawBitmap(fBitmap, fBitmap->Bounds(),
			rect, B_FILTER_BITMAP_BILINEAR);
	}

	// Draw contact name
	owner->MovePenTo(frame.left + 48, frame.top + fBaselineOffset);
	owner->SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
	owner->DrawString(Text());

	// Draw contact status string
	owner->MovePenTo(frame.left + 48, frame.top + fBaselineOffset +
		fBaselineOffset + 3);
	owner->SetHighColor(tint_color(lowColor, B_DARKEN_2_TINT));
	if (fPersonalStatus.Length() == 0)
		owner->DrawString(UserStatusToString(fStatus));
	else
		owner->DrawString(fPersonalStatus);

	// Draw separator between items
	owner->StrokeLine(BPoint(frame.left, frame.bottom),
		BPoint(frame.right, frame.bottom));

	// Draw protocol bitmpap
	BBitmap* protocolBitmap = contactLinker->ProtocolBitmap();

	if (protocolBitmap != NULL) {
		BRect rect(frame.right - 19, frame.top + 2,
			frame.right - 2, frame.top + 19 );;
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		owner->DrawBitmap(protocolBitmap, protocolBitmap->Bounds(),
			rect, B_FILTER_BITMAP_BILINEAR);
	}

	owner->SetHighColor(highColor);
	owner->SetLowColor(lowColor);
}


void
RosterItem::Update(BView* owner, const BFont* font)
{
	font_height fheight;
	font->GetHeight(&fheight);

	fBaselineOffset = 2 + ceilf(fheight.ascent + fheight.leading / 2);

	SetHeight((ceilf(fheight.ascent) + ceilf(fheight.descent) +
		ceilf(fheight.leading) + 4 ) * 2);
}


void
RosterItem::SetStatus(UserStatus status)
{
	if (fStatus != status)
		fStatus = status;
}
