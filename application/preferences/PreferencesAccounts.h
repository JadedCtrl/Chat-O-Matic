/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_ACCOUNTS_H
#define _PREFERENCES_ACCOUNTS_H

#include <View.h>

class BListView;
class BMenu;
class BButton;

class PreferencesAccounts : public BView {
public:
					PreferencesAccounts();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:
	BListView*		fListView;
	BMenu*			fProtosMenu;
	BButton*		fDelButton;
	BButton*		fEditButton;
};

#endif	// _PREFERENCES_ACCOUNTS_H
