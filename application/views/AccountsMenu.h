/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACCOUNTS_MENU_H
#define _ACCOUNTS_MENU_H

#include <Menu.h>

#include "Observer.h"


class AccountsMenu : public BMenu, public Observer {
public:
					AccountsMenu(const char* name, BMessage msg,
						BMessage* allMsg = NULL);
					~AccountsMenu();

	virtual void	ObserveInteger(int32 what, int32 value);

			void	SetDefaultSelection(BMenuItem* item);

private:
			void	_PopulateMenu();

	BMessage fAccountMessage;
	BMessage* fAllMessage;
	static int32 fDefaultSelection;
};

#endif // _ACCOUNTS_MENU_H
