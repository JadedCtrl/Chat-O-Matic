/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "AccountMenuItem.h"

#include "AccountsMenu.h"


AccountMenuItem::AccountMenuItem(const char* label, BMessage* msg,
	BBitmap* icon)
	:
	BitmapMenuItem(label, msg, icon, 0, 0, false)
{
}


void
AccountMenuItem::SetMarked(bool mark)
{
	BMenuItem::SetMarked(mark);
	if (mark == true)
		((AccountsMenu*)Menu())->SetDefaultSelection(this);
}
