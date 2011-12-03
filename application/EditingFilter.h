/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _EDITING_FILTER_H
#define _EDITING_FILTER_H

#include <MessageFilter.h>
#include <TextView.h>

class EditingFilter : public BMessageFilter {
public:
							EditingFilter(BTextView* view);

	virtual	filter_result	Filter(BMessage* message, BHandler** target);

private:
			BTextView*		fView;
};

#endif	// _EDITING_FILTER_H
