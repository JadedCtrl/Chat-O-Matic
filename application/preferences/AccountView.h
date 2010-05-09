/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_VIEW_H
#define _ACCOUNT_VIEW_H

#include <View.h>

const uint32 kChanged      = 'CHGD';

class AccountView : public BView {
public:
					AccountView(const char* name);

	virtual	void	AttachedToWindow();
};

#endif	// _ACCOUNT_VIEW_H
