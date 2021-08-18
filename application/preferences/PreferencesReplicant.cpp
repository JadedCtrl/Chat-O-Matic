/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "PreferencesReplicant.h"

#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>

#include "AppPreferences.h"
#include "ReplicantStatusView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesReplicant"


const uint32 kDisableReplicant = 'DSrp';
const uint32 kPermanentReplicant ='PRpt';
const uint32 kHideDeskbar = 'HCtk';


PreferencesReplicant::PreferencesReplicant()
	: BView(B_TRANSLATE("Replicant"), B_WILL_DRAW)
{
	BBox* replicantBox = new BBox("replicantBox");
	replicantBox->SetLabel(B_TRANSLATE("Deskbar replicant"));

	fDisableReplicant = new BCheckBox("DisableReplicant",
		B_TRANSLATE("Disable deskbar replicant"),
		new BMessage(kDisableReplicant));
	fDisableReplicant->SetEnabled(false); // Replicant is broken currently

	if (!AppPreferences::Get()->HideDeskbar)
		Looper()->PostMessage(new BMessage(kDisableReplicant));

	fPermanentReplicant = new BCheckBox("PermanentReplicant",
		B_TRANSLATE("Permanent deskbar replicant"), NULL);
	fPermanentReplicant->SetEnabled(false);

	fHideDeskbar = new BCheckBox("HideDeskbar",
		B_TRANSLATE("Hide field in Deskbar"), new BMessage(kHideDeskbar));
	fHideDeskbar->SetEnabled(false);


	const float spacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(replicantBox, B_VERTICAL)
		.SetInsets(spacing, spacing * 2, spacing, spacing)
		.Add(fDisableReplicant)
		.Add(fPermanentReplicant)
		.Add(fHideDeskbar)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(replicantBox)
		.AddGlue()
	.End();
}


void
PreferencesReplicant::AttachedToWindow()
{
	fHideDeskbar->SetTarget(this);
	fDisableReplicant->SetTarget(this);

	fHideDeskbar->SetValue(
		AppPreferences::Get()->HideDeskbar);
	fDisableReplicant->SetValue(
		AppPreferences::Get()->DisableReplicant);
}


void
PreferencesReplicant::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kHideDeskbar:
			AppPreferences::Get()->HideDeskbar
				= fHideDeskbar->Value();
			break;
		case kDisableReplicant:
			AppPreferences::Get()->DisableReplicant
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
