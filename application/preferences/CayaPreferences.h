/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PREFERENCES_H
#define _CAYA_PREFERENCES_H

#include "PreferencesContainer.h"

typedef struct CayaPreferencesData
{
	bool MoveToCurrentWorkspace;
	bool ActivateWindow;
	bool IgnoreEmoticons;

	CayaPreferencesData()
		:
		MoveToCurrentWorkspace(true),
		ActivateWindow(true),
		IgnoreEmoticons(false)
	{
	}
};

typedef PreferencesContainer<CayaPreferencesData> CayaPreferences;

#endif	// _CAYA_PREFERENCES_H
