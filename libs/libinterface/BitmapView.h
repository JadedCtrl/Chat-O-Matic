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
						BitmapView(const char* name, uint32 flags
							= B_WILL_DRAW);
						~BitmapView();

			status_t	InitCheck();

			BBitmap*	Bitmap() const;
			void		SetBitmap(const char* filename);
			void		SetBitmap(BBitmap* bitmap);

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
