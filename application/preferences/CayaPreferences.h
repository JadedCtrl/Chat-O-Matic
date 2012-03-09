/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PREFERENCES_H
#define _CAYA_PREFERENCES_H

#include "PreferencesContainer.h"

typedef struct _CayaPreferencesData
{
	bool MoveToCurrentWorkspace;
	bool FocusOnMessageReceived;
	bool FocusUserIsTyping;

	bool HideCayaDeskbar;
	bool DisableReplicant;

	bool IgnoreEmoticons;

	bool NotifyProtocolStatus;
	bool NotifyContactStatus;

	_CayaPreferencesData()
		:
		MoveToCurrentWorkspace(true),
		FocusOnMessageReceived(false),
		FocusUserIsTyping(false),
		HideCayaDeskbar(false),
		DisableReplicant(true),
		IgnoreEmoticons(false),
		NotifyProtocolStatus(true),
		NotifyContactStatus(false)
	{
	}
} CayaPreferencesData;

typedef PreferencesContainer<CayaPreferencesData> CayaPreferences;

#endif	// _CAYA_PREFERENCES_H
