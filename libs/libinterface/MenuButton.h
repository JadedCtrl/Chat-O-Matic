/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _MENU_BUTTON_H
#define _MENU_BUTTON_H

#include <Button.h>

class BPopUpMenu;


class MenuButton : public BButton {
public:
					MenuButton(const char* name, const char* label,
						BMessage* message);

	virtual void	MouseDown(BPoint where);
	virtual void	MouseUp(BPoint where);
	virtual void	MouseMoved(BPoint where, uint32 code, const BMessage* dragMsg);

	 BPopUpMenu*	Menu();
			void	SetMenu(BPopUpMenu* menu);

private:
	BPopUpMenu* fMenu;

};

#endif // _MENU_BUTTON_H
