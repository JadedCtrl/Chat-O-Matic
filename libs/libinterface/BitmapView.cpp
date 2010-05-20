/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Bitmap.h>
#include <LayoutUtils.h>
#include <TranslationUtils.h>

#include "BitmapView.h"

const float kMinWidth = 32.0f;
const float kMinHeight = 32.0f;


BitmapView::BitmapView(const char* name, uint32 flags)
	:
	BView(name, flags | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fBitmap(NULL),
	fWidth(kMinWidth),
	fHeight(kMinHeight)
{
	// Set transparent view color
	SetViewColor(B_TRANSPARENT_COLOR);
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


BBitmap*
BitmapView::Bitmap() const
{
	return fBitmap;
}


void
BitmapView::SetBitmap(BBitmap* bitmap)
{
	delete fBitmap;
	fBitmap = bitmap;

	if (fBitmap) {
		BRect frame(fBitmap->Bounds());

		fWidth = frame.Width();
		fHeight = frame.Height();

		Invalidate();
	}
}


BSize
BitmapView::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
		BSize(kMinWidth, kMinHeight));
}


BSize
BitmapView::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		BSize(fWidth, fHeight));
}


BSize
BitmapView::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(),
		BSize(fWidth, fHeight));
}


void
BitmapView::Draw(BRect frame)
{
	if (!fBitmap)
		return;

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	DrawBitmap(fBitmap, fBitmap->Bounds(),
		Bounds(), B_FILTER_BITMAP_BILINEAR);
}
