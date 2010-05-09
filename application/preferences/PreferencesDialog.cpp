/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Button.h>
#include <ControlLook.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <TabView.h>

#include "PreferencesDialog.h"
#include "PreferencesAccounts.h"

const uint32 kApply = 'SAVE';


PreferencesDialog::PreferencesDialog()
	: BWindow(BRect(0, 0, 400, 400), "Preferences", B_TITLED_WINDOW,
		B_NOT_RESIZABLE | B_NOT_ZOOMABLE | B_CLOSE_ON_ESCAPE)
{
	BTabView* tabView = new BTabView("tabView", B_WIDTH_AS_USUAL);
	tabView->AddTab(new PreferencesAccounts());

	BButton* ok = new BButton("OK", new BMessage(kApply));

	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_VERTICAL,
		be_control_look->DefaultItemSpacing()));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(tabView)
		.AddGroup(B_HORIZONTAL)
			.AddGlue()
			.Add(ok)
			.SetInsets(spacing, spacing, 0, 0)
		.End()
		.SetInsets(spacing, spacing, spacing, spacing)
	);

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
