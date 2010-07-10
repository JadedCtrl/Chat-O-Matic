/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "StripeView.h"

static const int kIconStripeWidth = 30;

StripeView::StripeView(BRect frame)
	:	BView(frame, "StripeView", B_FOLLOW_ALL_SIDES, B_WILL_DRAW),
	    fIconBitmap(NULL)
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


StripeView::StripeView(BMessage* archive)
	:	BView(archive),
	    fIconBitmap(NULL)
{
}


StripeView::~StripeView() {}


StripeView*
StripeView::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "StripeView"))
		return NULL;

	return new StripeView(archive);
}


status_t
StripeView::Archive(BMessage* archive, bool deep)
{
	return BView::Archive(archive, deep);
}


void
StripeView::Draw(BRect updateRect)
{
	// Here's the fun stuff
	if (fIconBitmap) {
		BRect StripeRect = Bounds();
		StripeRect.right = kIconStripeWidth;
		SetHighColor(tint_color(ViewColor(), B_DARKEN_1_TINT));
		FillRect(StripeRect);


		SetDrawingMode(B_OP_ALPHA);
		DrawBitmapAsync(fIconBitmap, BPoint(18, 6));
		SetDrawingMode(B_OP_COPY);
	}
}
