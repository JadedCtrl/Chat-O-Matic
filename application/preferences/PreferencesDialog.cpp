/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Button.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <TabView.h>

#include "PreferencesDialog.h"
#include "PreferencesAccounts.h"
#include "PreferencesBehavior.h"
#include "PreferencesChatWindow.h"
#include "PreferencesReplicant.h"

const uint32 kApply = 'SAVE';


PreferencesDialog::PreferencesDialog()
	: BWindow(BRect(0, 0, 500, 615), "Preferences", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_CLOSE_ON_ESCAPE)
{
	BTabView* tabView = new BTabView("tabView", B_WIDTH_AS_USUAL);
	tabView->AddTab(new PreferencesAccounts());
	tabView->AddTab(new PreferencesBehavior());
	tabView->AddTab(new PreferencesChatWindow());
	tabView->AddTab(new PreferencesReplicant());

	BButton* ok = new BButton("OK", new BMessage(kApply));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(tabView)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(ok)
		.End();

	CenterOnScreen();
}


void
PreferencesDialog::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kApply:
			Close();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


