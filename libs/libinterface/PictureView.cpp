/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Bitmap.h>
#include <TranslationUtils.h>

#include "PictureView.h"


PictureView::PictureView(const char* name, const char* filename, uint32 flags)
	: BView(name, flags),
	fBitmap(NULL),
	fWidth(0.0f),
	fHeight(0.0f)
{
	// Set transparent
	SetViewColor(B_TRANSPARENT_COLOR);

	// Try to get the image
	fBitmap = BTranslationUtils::GetBitmap(filename);

	if (fBitmap) {
		BRect frame(fBitmap->Bounds());

		fWidth  = frame.Width();
		fHeight = frame.Height();
	} else
		return;

	ResizeTo(fWidth, fHeight);
}


PictureView::~PictureView()
{
	delete fBitmap;
	fBitmap = NULL;
}


status_t
PictureView::InitCheck()
{
	if (fBitmap != NULL)
		return B_OK;
	return B_ERROR;
}


BSize
PictureView::MinSize()
{
	return BSize(fWidth, fHeight);
}


BSize
PictureView::MaxSize()
{
	return MinSize();
}


BSize
PictureView::PreferredSize()
{
	return MinSize();
}


void
PictureView::Draw(BRect frame)
{
	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	if (fBitmap)
		DrawBitmap(fBitmap, BPoint(0, 0));
}
