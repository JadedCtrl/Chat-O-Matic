/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Bitmap.h>
#include <TranslationUtils.h>

#include "BitmapView.h"


BitmapView::BitmapView(const char* name, uint32 flags)
	: BView(name, flags),
	fBitmap(NULL),
	fWidth(0.0f),
	fHeight(0.0f)
{
	// Set transparent
	//SetViewColor(B_TRANSPARENT_COLOR);
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
}


BitmapView::~BitmapView()
{
	delete fBitmap;
	fBitmap = NULL;
}


status_t
BitmapView::InitCheck()
{
	if (fBitmap != NULL)
		return B_OK;
	return B_ERROR;
}


void
BitmapView::SetBitmap(BBitmap* bitmap)
{
	delete fBitmap;
	fBitmap = bitmap;

	if (fBitmap != NULL) {
		BRect frame(fBitmap->Bounds());

		fWidth  = frame.Width();
		fHeight = frame.Height();

		ResizeTo(fWidth, fHeight);
	}
}


BSize
BitmapView::MinSize()
{
	return BSize(fWidth, fHeight);
}


BSize
BitmapView::MaxSize()
{
	return MinSize();
}


BSize
BitmapView::PreferredSize()
{
	return MinSize();
}


void
BitmapView::Draw(BRect frame)
{
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	if (fBitmap != NULL)
		DrawBitmap(fBitmap, BPoint(0, 0));
}
