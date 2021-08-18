/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "PreferencesBehavior.h"

#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>

#include "AppPreferences.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesBehavior"


const uint32 kToCurrentWorkspace = 'CBcw';
const uint32 kRaiseOnMessageReceived = 'FCmr';
const uint32 kRaiseUserIsTyping = 'FCit';
const uint32 kMarkUnreadWindow = 'MKuw';
const uint32 kHideOffline = 'HiOf';
const uint32 kDisablePrompt = 'DiPr';


PreferencesBehavior::PreferencesBehavior()
	: BView(B_TRANSLATE("Behavior"), B_WILL_DRAW)
{
	BBox* incomingBox = new BBox("incoming");
	incomingBox->SetLabel(B_TRANSLATE("On incoming" B_UTF8_ELLIPSIS));
	
	fHideOffline = new BCheckBox("HideOfflineContacts",
		B_TRANSLATE("Hide offline contacts"),
		new BMessage(kHideOffline));
	fHideOffline->SetEnabled(false); //not implemented as yet

	fToCurrentWorkspace = new BCheckBox("ToCurrentWorkspace",
		B_TRANSLATE("Move window to current workspace"),
		new BMessage(kToCurrentWorkspace));
	fToCurrentWorkspace->SetEnabled(false); // not this either

	fRaiseOnMessageReceived = new BCheckBox("FocusOnMessageReceived",
		B_TRANSLATE("Auto-raise when a message is received"),
		new BMessage(kRaiseOnMessageReceived));
	fRaiseOnMessageReceived->SetEnabled(false); // nor this

	fMarkUnreadWindow = new BCheckBox("MarkUnreadWindow",
		B_TRANSLATE("Mark unread window chat"),
		new BMessage(kMarkUnreadWindow));
	fMarkUnreadWindow->SetEnabled(false); // of unimplemented settings!

	BBox* generalBox = new BBox("general");
	generalBox->SetLabel(B_TRANSLATE("General"));

	fDisableQuitConfirm = new BCheckBox("DisableQuitConfirm",
		B_TRANSLATE("Don't ask confirmation at Quit"),
		new BMessage(kDisablePrompt));
	const float spacing = be_control_look->DefaultItemSpacing();


	BLayoutBuilder::Group<>(generalBox, B_VERTICAL)
		.SetInsets(spacing, spacing * 2, spacing, spacing)
		.Add(fDisableQuitConfirm)
	.End();

	BLayoutBuilder::Group<>(incomingBox, B_VERTICAL)
		.SetInsets(spacing, spacing * 2, spacing, spacing)
		.Add(fHideOffline)
		.Add(fToCurrentWorkspace)
		.Add(fRaiseOnMessageReceived)
		.Add(fMarkUnreadWindow)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(generalBox)
		.Add(incomingBox)
		.AddGlue()
	.End();
}


void
PreferencesBehavior::AttachedToWindow()
{
	fHideOffline->SetTarget(this);
	fToCurrentWorkspace->SetTarget(this);
	fRaiseOnMessageReceived->SetTarget(this);
	fDisableQuitConfirm->SetTarget(this);
	
	fHideOffline->SetValue(
		AppPreferences::Get()->HideOffline);
	fToCurrentWorkspace->SetValue(
		AppPreferences::Get()->MoveToCurrentWorkspace);
	fRaiseOnMessageReceived->SetValue(
		AppPreferences::Get()->RaiseOnMessageReceived);
	fMarkUnreadWindow->SetValue(
		AppPreferences::Get()->MarkUnreadWindow);
	fDisableQuitConfirm->SetValue(
		AppPreferences::Get()->DisableQuitConfirm);
}


void
PreferencesBehavior::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kHideOffline:
			AppPreferences::Get()->HideOffline
				= fHideOffline->Value();
			break;
		case kToCurrentWorkspace:
			AppPreferences::Get()->MoveToCurrentWorkspace
				= fToCurrentWorkspace->Value();
			break;
		case kRaiseOnMessageReceived:
			AppPreferences::Get()->RaiseOnMessageReceived
				= fRaiseOnMessageReceived->Value();
			break;
		case kMarkUnreadWindow:
			AppPreferences::Get()->MarkUnreadWindow
				= fMarkUnreadWindow->Value();
			break;
		case kDisablePrompt:
			AppPreferences::Get()->DisableQuitConfirm
				= fDisableQuitConfirm->Value();
			break;
		default:
			BView::MessageReceived(message);
	}
}
