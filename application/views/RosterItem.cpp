/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <stdio.h>

#include <ListView.h>

#include "CayaUtils.h"
#include "ContactLinker.h"
#include "NotifyMessage.h"
#include "RosterItem.h"


RosterItem::RosterItem(const char*  name, ContactLinker* contact)
	: BStringItem(name),
	fBitmap(NULL),
	fStatus(CAYA_OFFLINE),
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
	if (fBitmap != NULL)
		delete fBitmap;
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
			SetStatus((CayaStatus)val);
			break;
	}
}


void RosterItem::DrawItem(BView* owner, BRect frame, bool complete)
{
	if (Text() == NULL)
	       return;

	rgb_color highlightColor = ui_color(B_CONTROL_HIGHLIGHT_COLOR);
    rgb_color highColor = owner->HighColor();
    rgb_color lowColor = owner->LowColor();

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
		case CAYA_ONLINE:
			owner->SetHighColor(CAYA_GREEN_COLOR);
			break;
		case CAYA_EXTENDED_AWAY:
		case CAYA_AWAY:
			owner->SetHighColor(CAYA_ORANGE_COLOR);
			break;
		case CAYA_DO_NOT_DISTURB:
			owner->SetHighColor(CAYA_RED_COLOR);
			break;
		case CAYA_OFFLINE:
			break;
		default:
	   		break;
	}
    owner->FillEllipse(BRect(frame.left + 6, frame.top + 6,
		frame.left + 6 + 7 , frame.top + 6 + 7));

	// Draw contact name
    owner->MovePenTo(frame.left + 20, frame.top + myfBaselineOffset);
    owner->SetHighColor(ui_color(B_CONTROL_TEXT_COLOR));
	owner->DrawString(Text());

	// Draw contact status string
    owner->MovePenTo(frame.left + 20, frame.top + myfBaselineOffset +
		myfBaselineOffset + 2);
	owner->SetHighColor(tint_color(lowColor, B_DARKEN_1_TINT));
    if (fPersonalStatus.Length() == 0)
		owner->DrawString(CayaStatusToString(fStatus));
	else
		owner->DrawString(fPersonalStatus);

	// Draw separator between items
    owner->StrokeLine(BPoint(frame.left, frame.bottom), BPoint(frame.right, frame.bottom));

	// Draw avatar icon
	if (fBitmap != NULL) {
		float h = frame.Height() - 4;
		BRect rect(frame.right - h - 2, frame.top + 2, frame.right - 2, frame.top + h );
		owner->SetDrawingMode(B_OP_ALPHA);
		owner->SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);
		owner->DrawBitmap(fBitmap, rect);
	}

    owner->SetHighColor(highColor);
	owner->SetLowColor(lowColor);
}


void
RosterItem::Update(BView* owner, const BFont* font)
{
	font_height fheight;
	font->GetHeight(&fheight);

	myfBaselineOffset = 2 + ceilf(fheight.ascent + fheight.leading / 2);

	SetHeight((ceilf(fheight.ascent) + ceilf(fheight.descent) +
		ceilf(fheight.leading) + 4 ) * 2);
}


void
RosterItem::SetStatus(CayaStatus status)
{
	if (fStatus != status)
		fStatus = status;
}
