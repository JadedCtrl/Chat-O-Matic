/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _BITMAP_VIEW_H
#define _BITMAP_VIEW_H

#include <View.h>

class BBitmap;

class BitmapView : public BView {
public:
						BitmapView(const char* name = NULL, uint32 flags
							= B_WILL_DRAW);
						~BitmapView();

	virtual	void		AttachedToWindow();

			status_t	InitCheck();

			BBitmap*	Bitmap() const;

			status_t	SetBitmap(const char* filename);
			status_t	SetBitmap(const BBitmap* bitmap);

	virtual BSize		MinSize();
	virtual BSize		MaxSize();
	virtual BSize		PreferredSize();

	virtual	void		Draw(BRect frame);

private:
			BBitmap*	fBitmap;
			float		fWidth;
			float		fHeight;
};

#endif	// _BITMAP_VIEW_H
