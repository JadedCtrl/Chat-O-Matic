/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PICTURE_VIEW_H
#define _PICTURE_VIEW_H

#include <View.h>

class BBitmap;

class PictureView : public BView {
public:
						PictureView(const char* name, const char* filename,
									uint32 flags = B_WILL_DRAW);
						~PictureView();

			status_t	InitCheck();

	virtual BSize		MinSize();
	virtual BSize		MaxSize();
	virtual BSize		PreferredSize();

	virtual	void		Draw(BRect frame);	

private:
			BBitmap*	fBitmap;
			float		fWidth;
			float		fHeight;
};

#endif	// _PICTURE_VIEW_H
