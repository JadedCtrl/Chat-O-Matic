/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <new>

#include <Bitmap.h>
#include <LayoutUtils.h>
#include <TranslationUtils.h>

#include "BitmapView.h"

const float kMinWidth	= 32.0f;
const float kMinHeight	= 32.0f;


BitmapView::BitmapView(const char* name, uint32 flags)
	:
	BView(name, flags | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fBitmap(NULL),
	fIsSquare(false),
	fWidth(kMinWidth),
	fHeight(kMinHeight)
{
}


BitmapView::~BitmapView()
{
	delete fBitmap;
	fBitmap = NULL;
}


void
BitmapView::AttachedToWindow()
{
	// Set view color to parent's view color
	if (Parent() != NULL)
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));
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


status_t
BitmapView::SetBitmap(const char* filename)
{
	delete fBitmap;

	fBitmap = BTranslationUtils::GetBitmap(filename);
	if (fBitmap == NULL)
		return B_ERROR;

	return B_OK;
}


status_t
BitmapView::SetBitmap(const BBitmap* bitmap)
{
	delete fBitmap;
	fBitmap = NULL;

	if (bitmap != NULL) {
		fBitmap = new(std::nothrow) BBitmap(bitmap);
		if (fBitmap == NULL)
			return B_NO_MEMORY;
		if (fBitmap->InitCheck() != B_OK)
			return fBitmap->InitCheck();

		fWidth = fBitmap->Bounds().Width();
		fHeight = fBitmap->Bounds().Height();
		Invalidate();
	}

	return B_OK;
}


void
BitmapView::SetSquare(bool isSquare)
{
	fIsSquare = isSquare;
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
	if (fBitmap == NULL)
		return;

	SetDrawingMode(B_OP_ALPHA);
	SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

	DrawBitmap(fBitmap, fBitmap->Bounds(),
		_ViewBounds(), B_FILTER_BITMAP_BILINEAR);
}


BRect
BitmapView::_ViewBounds()
{
	BRect bounds = Bounds();
	if (fIsSquare == false || bounds.Height() == bounds.Width())
		return bounds;

	BPoint lt = bounds.LeftTop();
	BPoint rb = bounds.RightBottom();
	float diff = 0.0;

	if (bounds.Height() > bounds.Width()) {
		diff = bounds.Height() - bounds.Width();
		lt -= BPoint(0.0, diff / 2);
		rb += BPoint(0.0, diff / 2);
	}
	else if (bounds.Width() > bounds.Height()) {
		diff = bounds.Height() - bounds.Width();
		lt -= BPoint(diff / 2, 0.0);
		rb += BPoint(diff / 2, 0.0);
	}
	return BRect(lt, rb);
}
