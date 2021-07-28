/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACCOUNT_MENU_ITEM_H
#define _ACCOUNT_MENU_ITEM_H

#include <MenuItem.h>


class AccountMenuItem : public BMenuItem {
public:
					AccountMenuItem(const char* label, BMessage* msg);
	
	virtual void	SetMarked(bool mark);
};

#endif // _ACCOUNT_MENU_ITEM_H
