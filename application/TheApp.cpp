/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
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

#include "Caya.h"
#include "TheApp.h"
#include "FilePanel.h"
#include "MainWindow.h"
#include "Emoticor.h"
#include "ProtocolManager.h"


TheApp::TheApp()
	: BApplication(CAYA_SIGNATURE),
	fMainWin(NULL)
{
}


void	
TheApp::ReadyToRun()
{	
	app_info theInfo;

	if (be_app->GetAppInfo(&theInfo) == B_OK) {
		BPath applicationDirectory(&theInfo.ref);
		applicationDirectory.GetParent(&applicationDirectory);

		BPath currentPath = applicationDirectory;
		currentPath.Append("smileys");
		currentPath.Append("settings.xml");
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

		currentPath = applicationDirectory;
		currentPath.Append("protocols");
		if (BEntry(currentPath.Path()).Exists()) {
			printf("Looking for protocols from: %s\n", currentPath.Path());
			ProtocolManager::Get()->Init(BDirectory(currentPath.Path()));
		} else {
			BString msg("Can't find protocols in:\n\n");
			msg << currentPath.Path();
			BAlert* alert = new BAlert("", msg.String(), "Ouch!");
			alert->Go();
			PostMessage(B_QUIT_REQUESTED);
			return;
		}	
	}

	fMainWin = new MainWindow();
	fMainWin->Show();
}


MainWindow*
TheApp::GetMainWindow() const
{
	return fMainWin;
}
