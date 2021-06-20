/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2011-2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_MENU_ITEM_H
#define _STATUS_MENU_ITEM_H

#include <MenuItem.h>

#include <libinterface/BitmapMenuItem.h>

#include "AppConstants.h"

class BBitmap;

const int32 kSetStatus = 'SEST';

class ReplicantMenuItem : public BitmapMenuItem {
public:
				ReplicantMenuItem(const char* label, UserStatus status,
					bool custom = false, char shortcut = 0, uint32 modifiers = 0);

	UserStatus	Status() const;
	bool		IsCustom() const;

private:
	UserStatus	fStatus;
	bool		fCustom;

	void		SetIcon();
};

#endif	// _STATUS_MENU_ITEM_H
