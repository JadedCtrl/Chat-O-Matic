/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "MenuButton.h"

#include <PopUpMenu.h>


MenuButton::MenuButton(const char* name, const char* label, BMessage* message)
	:
	BButton(name, label, message),
	fMenu(NULL)
{
	SetBehavior(B_POP_UP_BEHAVIOR);
}


void
MenuButton::MouseDown(BPoint where)
{
	BButton::MouseDown(where);
	if (fMenu != NULL)
		fMenu->Go(ConvertToScreen(Bounds().LeftBottom()), true, true, true);
}


void
MenuButton::MouseUp(BPoint where)
{
	BButton::MouseUp(where);
	SetValue(B_CONTROL_OFF);
}


void
MenuButton::MouseMoved(BPoint where, uint32 code, const BMessage* dragMsg)
{
}



BPopUpMenu*
MenuButton::Menu()
{
	return fMenu;
}


void
MenuButton::SetMenu(BPopUpMenu* menu)
{
	fMenu = menu;
}
