/*
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SEARCHBAR_TEXT_CONTROL_H
#define _SEARCHBAR_TEXT_CONTROL_H

#include <TextControl.h>

class SearchBarTextControl : public BTextControl {
public:
					SearchBarTextControl(BMessage* message);

	virtual void	KeyDown(const char* bytes, int32 numBytes);
};

#endif	// _SEARCHBAR_TEXT_CONTROL_H
