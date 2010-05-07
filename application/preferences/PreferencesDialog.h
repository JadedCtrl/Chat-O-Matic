/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_DIALOG_H
#define _PREFERENCES_DIALOG_H

#include <Window.h>

class PreferencesDialog : public BWindow {
public:
					PreferencesDialog();

	virtual	void	MessageReceived(BMessage* msg);
};

#endif	// _PREFERENCES_DIALOG_H
