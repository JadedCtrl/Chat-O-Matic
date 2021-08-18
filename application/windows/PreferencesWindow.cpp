/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "PreferencesWindow.h"

#include <Button.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <TabView.h>

#include "PreferencesBehavior.h"
#include "PreferencesChatWindow.h"
#include "PreferencesNotifications.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesWindow"


const uint32 kApply = 'SAVE';


PreferencesWindow::PreferencesWindow()
	: BWindow(BRect(0, 0, 500, 615), B_TRANSLATE("Preferences"),
		B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_RESIZABLE
			| B_NOT_ZOOMABLE | B_CLOSE_ON_ESCAPE)
{
	BTabView* tabView = new BTabView("tabView", B_WIDTH_AS_USUAL);
	tabView->AddTab(new PreferencesBehavior());
	tabView->AddTab(new PreferencesChatWindow());
	tabView->AddTab(new PreferencesNotifications());

	// Tab resizing here is a bit wonky. We want each tab to be visible,
	// but we don't want the tab-view to be too wide…
	float charCount = 0;
	for (int i = 0; i < tabView->CountTabs(); i++)
		charCount += strlen(tabView->TabAt(i)->Label());

	// These values account for the decreasing amount of padding within tabs,
	// Ignucius forgive me.
	float textWidth = be_plain_font->Size();
	switch ((int)textWidth) {
		case 8:  case 9:	charCount += 14; break;
		case 10: case 11:	charCount += 5;  break;
		case 12: break;
		case 13: case 14: case 15:
							charCount -= 4;  break;
		default:			charCount -= 10;
	}
	tabView->SetExplicitMinSize(BSize(charCount * textWidth, B_SIZE_UNSET));

	BButton* ok = new BButton(B_TRANSLATE("OK"), new BMessage(kApply));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(tabView)
		.AddGroup(B_HORIZONTAL)
		.SetInsets(0, 0, B_USE_HALF_ITEM_SPACING, B_USE_HALF_ITEM_SPACING)
			.AddGlue()
			.Add(ok)
		.End()
	.End();

	CenterOnScreen();
}


void
PreferencesWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kApply:
			Close();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


