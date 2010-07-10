/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef 	_StripeView_H_
#define	_StripeView_H_

#include <View.h>

class StripeView : public BView
{
public:
	StripeView(BRect frame);
	StripeView(BMessage* archive);
	~StripeView();

	static StripeView*	Instantiate(BMessage* archive);
	status_t			Archive(BMessage* archive, bool deep = true);

	virtual void	Draw(BRect updateRect);

	// These functions (or something analogous) are missing from libbe.so's
	// dump.  I can only assume that the bitmap is a public var in the
	// original implementation -- or BPAlert is a friend of StripeView.
	// Neither one is necessary, since I can just add these.
	void			SetBitmap(BBitmap* Icon) {
		fIconBitmap = Icon;
	}
	BBitmap*		Bitmap() {
		return fIconBitmap;
	}

private:
	BBitmap*	fIconBitmap;
};

#endif
