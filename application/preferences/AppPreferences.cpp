/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "AppPreferences.h"

#include "Utils.h"


AppPreferences* AppPreferences::fInstance = NULL;


AppPreferences*
AppPreferences::Get()
{
	if (fInstance == NULL) {
		fInstance = new AppPreferences();
		fInstance->Load();
	}
	return fInstance;
}


void
AppPreferences::Load()
{
	const char* path = _PreferencesPath();
	BFile file(_PreferencesPath(), B_READ_ONLY);
	BMessage settings;

	if (file.InitCheck() == B_OK)
		settings.Unflatten(&file);

	MoveToCurrentWorkspace = settings.GetBool("MoveToCurrentWorkpace", false);
	RaiseOnMessageReceived = settings.GetBool("RaiseOnMessageReceived", false);
	RaiseUserIsTyping = settings.GetBool("RaiseUserIsTyping", false);
	MarkUnreadWindow = settings.GetBool("MarkUnreadWindow", true);
	NotifyProtocolStatus = settings.GetBool("NotifyProtocolStatus", true);
	NotifyNewMessage = settings.GetBool("NotifyNewMessage", true);
	NotifyContactStatus = settings.GetBool("NotifyContactStatus", false);
	HideDeskbar = settings.GetBool("HideDeskbar", false);
	DisableReplicant = settings.GetBool("DisableReplicant", true);
	DisableQuitConfirm = settings.GetBool("DisableQuitConfirm", false);
	IgnoreEmoticons = settings.GetBool("IgnoreEmoticons", true);
	HideOffline = settings.GetBool("HideOffline", false);

	MainWindowListWeight = settings.GetFloat("MainWindowListWeight", 1);
	MainWindowChatWeight = settings.GetFloat("MainWindowChatWeight", 5);

	MainWindowRect = settings.GetRect("MainWindowRect", BRect(0, 0, 600, 400));
}


void
AppPreferences::Save()
{
	const char* path = _PreferencesPath();
	BFile file(_PreferencesPath(), B_WRITE_ONLY | B_CREATE_FILE);

	BMessage settings;
	settings.AddBool("MoveToCurrentWorkpace", MoveToCurrentWorkspace);
	settings.AddBool("RaiseOnMessageReceived", RaiseOnMessageReceived);
	settings.AddBool("RaiseUserIsTyping", RaiseUserIsTyping);
	settings.AddBool("MarkUnreadWindow", MarkUnreadWindow);
	settings.AddBool("NotifyProtocolStatus", NotifyProtocolStatus);
	settings.AddBool("NotifyNewMessage", NotifyNewMessage);
	settings.AddBool("NotifyContactStatus", NotifyContactStatus);
	settings.AddBool("HideDeskbar", HideDeskbar);
	settings.AddBool("DisableReplicant", DisableReplicant);
	settings.AddBool("DisableQuitConfirm", DisableQuitConfirm);
	settings.AddBool("IgnoreEmoticons", IgnoreEmoticons);
	settings.AddBool("HideOffline", HideOffline);

	settings.AddFloat("MainWindowListWeight", MainWindowListWeight);
	settings.AddFloat("MainWindowChatWeight", MainWindowChatWeight);

	settings.AddRect("MainWindowRect", MainWindowRect);

	if (file.InitCheck() == B_OK)
		settings.Flatten(&file);
}


const char*
AppPreferences::_PreferencesPath()
{
	BPath path(SettingsPath());
	path.Append("preferences");
	return path.Path();
}
