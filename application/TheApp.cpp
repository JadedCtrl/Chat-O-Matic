/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <stdio.h>

#include <Alert.h>
#include <Path.h>
#include <Roster.h>

#include "AboutWindow.h"
#include "Caya.h"
#include "CayaMessages.h"
#include "Emoticor.h"
#include "FilePanel.h"
#include "MainWindow.h"
#include "ProtocolManager.h"
#include "ReplicantStatusView.h"
#include "Server.h"
#include "TheApp.h"

TheApp::TheApp()
	:
	BApplication(CAYA_SIGNATURE),
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
			BString msg("Can't find smileys settings in:\n\n");
			msg << currentPath.Path();
			BAlert* alert = new BAlert("", msg.String(), "Ouch!");
			alert->Go();
		}
		printf("Loaded Emoticons settings from: %s\n", currentPath.Path());

		currentPath = appDir;
		currentPath.Append("protocols");
		if (BEntry(currentPath.Path()).Exists()) {
			printf("Looking for protocols from: %s\n", currentPath.Path());

			ProtocolManager::Get()->Init(BDirectory(currentPath.Path()),
				fMainWin);
		} else {
			BString msg("Can't find protocols in:\n\n");
			msg << currentPath.Path();
			BAlert* alert = new BAlert("", msg.String(), "Ouch!");
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

	const char* authors[] = {
		"Andrea Anzani",
		"Dario Casalinuovo",
		"Pier Luigi Fiorini",
		NULL
	};

	BString extraInfo;
	extraInfo << "Caya is released under the GNU GPL License." << "\n";
	extraInfo << "Some parts of Caya are available under MIT license." << "\n";
	extraInfo << "Built: " << BUILD_DATE;

	AboutWindow* about = new AboutWindow("Caya", holders,
		authors, extraInfo.String());
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
		case CAYA_REPLICANT_STATUS_SET:
		case CAYA_REPLICANT_SHOW_WINDOW:
		case CAYA_SHOW_SETTINGS:
		case CAYA_REPLICANT_MESSENGER:
			DetachCurrentMessage();
			fMainWin->PostMessage(message);
			break;
		case CAYA_REPLICANT_EXIT:
			// TODO BAlert here
			PostMessage(B_QUIT_REQUESTED);
			break;
		default:
			BLooper::MessageReceived(message);
	}
}
