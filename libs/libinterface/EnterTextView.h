/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ENTER_TEXT_VIEW_H
#define _ENTER_TEXT_VIEW_H

#include <TextView.h>


class EnterTextView : public BTextView {
public:
					EnterTextView(const char* name);
					EnterTextView(const char* name, const BFont* initialFont,
						const rgb_color* initialColor,
						uint32 flags = B_WILL_DRAW);

	virtual void	KeyDown(const char* bytes, int32 numBytes);

			void	SetTarget(BHandler* handler) { fTarget = handler; }

		BMessage	Message() { return fMessage; }
			void	SetMessage(BMessage msg, const char* textSlot = NULL);

private:
	BMessage fMessage;
	BHandler* fTarget;
	BString fTextSlot;
};

#endif // _ENTER_TEXT_VIEW_H
