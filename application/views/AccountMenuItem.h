/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACCOUNT_MENU_ITEM_H
#define _ACCOUNT_MENU_ITEM_H

#include <MenuItem.h>

#include <libinterface/BitmapMenuItem.h>


class AccountMenuItem : public BitmapMenuItem {
public:
					AccountMenuItem(const char* label, BMessage* msg, BBitmap* icon = NULL);
	
	virtual void	SetMarked(bool mark);
};

#endif // _ACCOUNT_MENU_ITEM_H
