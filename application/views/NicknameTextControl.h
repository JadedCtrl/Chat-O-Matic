/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _NICKNAME_TEXT_CONTROL_H
#define _NICKNAME_TEXT_CONTROL_H

#include <TextView.h>

class NicknameTextControl : public BTextView {
public:
					NicknameTextControl(const char* name, BMessage* message);

//	virtual	void	Draw(BRect updateRect);
};

#endif	// _NICKNAME_TEXT_CONTROL_H
