/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "TheApp.h"

#include <stdio.h>

#include <AboutWindow.h>
#include <Alert.h>
#include <Catalog.h>
#include <Path.h>
#include <Roster.h>

#include <librunview/Emoticor.h>

#include "AppMessages.h"
#include "FilePanel.h"
#include "MainWindow.h"
#include "ProtocolManager.h"
#include "ReplicantStatusView.h"
#include "Server.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TheApp"


int
main(int argc, char* argv[])
{
	TheApp app;
	app.Run();

	return 0;
}


TheApp::TheApp()
	:
	BApplication(APP_SIGNATURE),
	fMainWin(NULL)
{
}


void	
TheApp::ReadyToRun()
{
	app_info theInfo;

	fMainWin = new MainWindow();

	if (be_app->GetAppInfo(&theInfo) == B_OK) {
		BPath appDir(&theInfo.ref);
		appDir.GetParent(&appDir);

		// Emoticons settings
		BPath currentPath = appDir;
		currentPath.Append("smileys");
		currentPath.Append("settings.xml");

		// Load emoticons
		BEntry entry(currentPath.Path());
		if (entry.Exists())
			Emoticor::Get()->LoadConfig(currentPath.Path());
		else {
			BString msg(B_TRANSLATE("Can't find smileys settings in:\n\n%path%"));
			msg.ReplaceAll("%path%", currentPath.Path());
			BAlert* alert = new BAlert("", msg.String(), B_TRANSLATE("Ouch!"));
//			alert->Go();
		}
		printf("Loaded Emoticons settings from: %s\n", currentPath.Path());

		bool win = false;
		if (_LoadProtocols(B_SYSTEM_ADDONS_DIRECTORY))				win = true;
		if (_LoadProtocols(B_USER_ADDONS_DIRECTORY))				win = true;
		if (_LoadProtocols(B_SYSTEM_NONPACKAGED_ADDONS_DIRECTORY))	win = true;
		if (_LoadProtocols(B_USER_NONPACKAGED_ADDONS_DIRECTORY))	win = true;
		if (_LoadProtocols(appDir))									win = true;

		if (win == false) {
			BString msg(B_TRANSLATE("No protocols found!\nPlease make sure %app% was installed correctly."));
			msg.ReplaceAll("%app%", APP_NAME);
			BAlert* alert = new BAlert("", msg.String(), B_TRANSLATE("Ouch!"));
			alert->Go();
			PostMessage(B_QUIT_REQUESTED);
			return;
		}
	}
	fMainWin->Start();
	fMainWin->Show();
}


void
TheApp::AboutRequested()
{
	const char* holders[] = {
		"2009-2010 Andrea Anzani",
		"2010-2015 Dario Casalinuovo",
		"2009-2010 Pier Luigi Fiorini",
		NULL
	};

	BString extraInfo(B_TRANSLATE("%app% is released under the MIT License.\n"
		"Add-on and library licenses may vary.\n"
		"Built: %buildDate%"));
	extraInfo.ReplaceAll("%buildDate%", BUILD_DATE);
	extraInfo.ReplaceAll("%app%", B_TRANSLATE_SYSTEM_NAME(APP_NAME));

	BAboutWindow* about = new BAboutWindow(B_TRANSLATE_SYSTEM_NAME(APP_NAME),
		APP_SIGNATURE);
	about->AddDescription(B_TRANSLATE("A multi-protocol chat program."));
	about->AddCopyright(2021, "Jaidyn Levesque", holders);
	about->AddExtraInfo(extraInfo);
	about->Show();
}


MainWindow*
TheApp::GetMainWindow() const
{
	return fMainWin;
}


void
TheApp::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case APP_REPLICANT_STATUS_SET:
		case APP_REPLICANT_SHOW_WINDOW:
		case APP_SHOW_SETTINGS:
		case APP_REPLICANT_MESSENGER:
			DetachCurrentMessage();
			fMainWin->PostMessage(message);
			break;
		case APP_REPLICANT_EXIT:
			// TODO BAlert here
			PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			BLooper::MessageReceived(message);
	}
}


bool
TheApp::_LoadProtocols(directory_which finddir)
{
	BPath path;
	if (find_directory(finddir, &path) == B_OK)
		return _LoadProtocols(path);
	return false;
}


bool
TheApp::_LoadProtocols(BPath path)
{
	path.Append(BString(APP_NAME).ToLower());
	if (BEntry(path.Path()).Exists()) {
		printf("Looking for protocols from: %s\n", path.Path());
		return ProtocolManager::Get()->Init(BDirectory(path.Path()), fMainWin);
	}
	return false;
}
