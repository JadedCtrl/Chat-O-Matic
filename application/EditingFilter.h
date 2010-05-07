/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef EDITING_FILTER_H
#define EDITING_FILTER_H

#include <MessageFilter.h>
#include <interface/TextView.h>

class EditingFilter : public BMessageFilter {
public:
	                      EditingFilter(BTextView *view);

	virtual filter_result Filter(BMessage *message, BHandler **target);

private:
	BTextView           *_view;
};
#endif

