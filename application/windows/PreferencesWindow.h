/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_WINDOW_H
#define _PREFERENCES_WINDOW_H

#include <Window.h>

class PreferencesWindow : public BWindow {
public:
					PreferencesWindow();

	virtual	void	MessageReceived(BMessage* msg);
};

#endif	// _PREFERENCES_WINDOW_H
