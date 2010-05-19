/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <Button.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <StringView.h>

#include "CayaProtocol.h"
#include "PreferencesBehavior.h"
#include "CayaPreferences.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "TheApp.h"

const uint32 kToCurrentWorkspace = 'CBcw';
const uint32 kActivateChatWindow = 'CBac';


PreferencesBehavior::PreferencesBehavior()
	: BView("Behavior", B_WILL_DRAW)
{

	fOnIncomming = new BStringView("onIncomming", "On incomming message ... ");
	fOnIncomming->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fOnIncomming->SetFont(be_bold_font);

	fToCurrentWorkspace = new BCheckBox("ToCurrentWorkspace",
		"Move window to current workspace",
		new BMessage(kToCurrentWorkspace));

	fActivateChatWindow = new BCheckBox("ActivateChatWindow",
		"Get focus ",
		new BMessage(kActivateChatWindow));

	fPlaySoundOnMessageReceived = new BCheckBox("PlaySoundOnMessageReceived",
		"Play sound event", NULL);
	fPlaySoundOnMessageReceived->SetEnabled(false);  // not implemented

	fHideEmoticons = new BCheckBox("HideEmoticons",
		"Hide Emoticons", NULL);
	fHideEmoticons->SetEnabled(false);  // not implemented

	fMarkUnreadWindow = new BCheckBox("MarkUnreadWindow",
		"Mark unread window chat", NULL);
	fMarkUnreadWindow->SetEnabled(false); // not implemented

	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_HORIZONTAL, spacing));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fOnIncomming)
		.AddGroup(B_VERTICAL, spacing)
			.Add(fToCurrentWorkspace)
			.Add(fActivateChatWindow)
			.Add(fMarkUnreadWindow)
			.Add(fPlaySoundOnMessageReceived)
		.SetInsets(spacing * 2, spacing, spacing, spacing)
		.End()
		.Add(fHideEmoticons)
		.AddGlue()
		.SetInsets(spacing, spacing, spacing, spacing)
	);
}


void
PreferencesBehavior::AttachedToWindow()
{
	fToCurrentWorkspace->SetTarget(this);
	fActivateChatWindow->SetTarget(this);

	fToCurrentWorkspace->SetValue(
		CayaPreferences::Item()->MoveToCurrentWorkspace);
	fActivateChatWindow->SetValue(
		CayaPreferences::Item()->ActivateWindow);
}


void
PreferencesBehavior::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kToCurrentWorkspace:
			CayaPreferences::Item()->MoveToCurrentWorkspace
				= fToCurrentWorkspace->Value();
			break;
		case kActivateChatWindow:
			CayaPreferences::Item()->ActivateWindow
				= fActivateChatWindow->Value();
			break;
		default:
			BView::MessageReceived(message);
	}
}
