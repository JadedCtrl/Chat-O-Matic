/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_ACCOUNTS_H
#define _PREFERENCES_ACCOUNTS_H

#include <View.h>

class BButton;
class BListView;
class BPopUpMenu;

class ProtocolSettings;


class PreferencesAccounts : public BView {
public:
					PreferencesAccounts();

	virtual	void	AttachedToWindow();
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

#endif	// _PREFERENCES_ACCOUNTS_H
