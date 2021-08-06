/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNTS_WINDOW_H
#define _ACCOUNTS_WINDOW_H

#include <Window.h>

class BButton;
class BListView;
class BPopUpMenu;

class ProtocolSettings;


class AccountsWindow : public BWindow {
public:
					AccountsWindow();

	virtual	void	MessageReceived(BMessage* msg);

private:
	BListView*		fListView;
	BPopUpMenu*		fProtosMenu;
	BButton*		fDelButton;
	BButton*		fEditButton;
	BButton*		fToggleButton;

	void			_LoadListView(ProtocolSettings* settings);

	void			_DisableAccount(const char* account, int64 instance);
	void			_EnableAccount(const char* account,
						ProtocolSettings* settings);

	int64			_AccountInstance(const char* account);
};

#endif	// _ACCOUNTS_WINDOW_H
