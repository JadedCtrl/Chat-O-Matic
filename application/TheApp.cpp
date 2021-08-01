/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "TheApp.h"

#include <stdio.h>

#include <Alert.h>
#include <Catalog.h>
#include <Path.h>
#include <Roster.h>

#include <librunview/Emoticor.h>

#include "AboutWindow.h"
#include "Cardie.h"
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

		currentPath = appDir;
		currentPath.Append("protocols");
		if (BEntry(currentPath.Path()).Exists()) {
			printf("Looking for protocols from: %s\n", currentPath.Path());

			ProtocolManager::Get()->Init(BDirectory(currentPath.Path()),
				fMainWin);
		} else {
			BString msg("Can't find protocols in:\n\n%path%");
			msg.ReplaceAll("%path%", currentPath.Path());
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
		"2021 Jaidyn Levesque",
		NULL
	};

	const char* authors[] = {
		"Andrea Anzani",
		"Dario Casalinuovo",
		"Pier Luigi Fiorini",
		"Jaidyn Levesque",
		NULL
	};

	BString extraInfo(B_TRANSLATE("%app% is released under the GNU GPL "
		"License.\nSome parts of %app% are available under MIT license.\n"
		"Built: %buildDate%"));
	extraInfo.ReplaceAll("%buildDate", BUILD_DATE);
	extraInfo.ReplaceAll("%app%", B_TRANSLATE_SYSTEM_NAME(APP_NAME));

	AboutWindow* about = new AboutWindow(B_TRANSLATE_SYSTEM_NAME(APP_NAME),
		holders, authors, extraInfo.String());
	about->Show();
	delete about;
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
