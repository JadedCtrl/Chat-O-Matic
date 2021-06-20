/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <CheckBox.h>
#include <ControlLook.h>
#include <Deskbar.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <StringView.h>

#include "AccountManager.h"
#include "ChatProtocol.h"
#include "PreferencesReplicant.h"
#include "AppPreferences.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "ReplicantStatusView.h"
#include "TheApp.h"

const uint32 kDisableReplicant = 'DSrp';
const uint32 kPermanentReplicant ='PRpt';
const uint32 kHideDeskbar = 'HCtk';


PreferencesReplicant::PreferencesReplicant()
	: BView("Replicant", B_WILL_DRAW)
{
	fReplicantString = new BStringView("ReplicantString",
		"Deskbar Replicant");

	fReplicantString->SetExplicitAlignment(
		BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));

	fReplicantString->SetFont(be_bold_font);

	fDisableReplicant = new BCheckBox("DisableReplicant",
		"Disable Deskbar replicant", new BMessage(kDisableReplicant));

	if (!AppPreferences::Item()->HideDeskbar)
		Looper()->PostMessage(new BMessage(kDisableReplicant));

	fPermanentReplicant = new BCheckBox("PermanentReplicant",
		"Permanent Deskbar Replicant", NULL);
	fPermanentReplicant->SetEnabled(false);

	fHideDeskbar = new BCheckBox("HideDeskbar",
		"Hide field in Deskbar", new BMessage(kHideDeskbar));
	fHideDeskbar->SetEnabled(false);

	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_HORIZONTAL, spacing));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fReplicantString)
		.AddGroup(B_VERTICAL, spacing)
			.Add(fDisableReplicant)
			.Add(fPermanentReplicant)
			.Add(fHideDeskbar)
			.SetInsets(spacing * 2, spacing, spacing, spacing)
		.End()
		.AddGlue()
		.SetInsets(spacing, spacing, spacing, spacing)
		.TopView()
	);
}


void
PreferencesReplicant::AttachedToWindow()
{
	fHideDeskbar->SetTarget(this);
	fDisableReplicant->SetTarget(this);

	fHideDeskbar->SetValue(
		AppPreferences::Item()->HideDeskbar);
	fDisableReplicant->SetValue(
		AppPreferences::Item()->DisableReplicant);
}


void
PreferencesReplicant::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kHideDeskbar:
			AppPreferences::Item()->HideDeskbar
				= fHideDeskbar->Value();
			break;
		case kDisableReplicant:
			AppPreferences::Item()->DisableReplicant
				= fDisableReplicant->Value();

			if (fDisableReplicant->Value() == true)
				ReplicantStatusView::RemoveReplicant();
			else
				ReplicantStatusView::InstallReplicant();

			break;
		default:
			BView::MessageReceived(message);
	}
}
