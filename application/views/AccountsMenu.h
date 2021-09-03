/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ACCOUNTS_MENU_H
#define _ACCOUNTS_MENU_H

#include <PopUpMenu.h>

#include "Observer.h"

class ProtocolLooper;
class Server;


class AccountsMenu : public BPopUpMenu, public Observer {
public:
					AccountsMenu(const char* name, BMessage msg,
						BMessage* allMsg, Server* server);
					AccountsMenu(const char* name, BMessage msg,
						BMessage* allMsg = NULL);
					~AccountsMenu();

	virtual void	ObserveInteger(int32 what, int32 value);

			void	SetDefaultSelection(BMenuItem* item);
	static int64	GetDefaultSelection() { return fDefaultSelection; }

private:
			void	_PopulateMenu();

		BBitmap*	_EnsureProtocolIcon(const char* label,
						ProtocolLooper* looper);
		BBitmap*	_EnsureAsteriskIcon();
			

	BMessage fAccountMessage;
	BMessage* fAllMessage;
	static int64 fDefaultSelection;
	Server* fServer;
};

#endif // _ACCOUNTS_MENU_H
