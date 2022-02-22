/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021-2022, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "PreferencesChatWindow.h"

#include <Box.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>

#include "AppPreferences.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesChatWindow"


const uint32 kIgnoreEmoticons = 'CBhe';
const uint32 kMembershipUpdates = 'CBmu';


PreferencesChatWindow::PreferencesChatWindow()
	: BView(B_TRANSLATE("Chat view"), B_WILL_DRAW)
{
	BBox* chatBox = new BBox("chatBox");
	chatBox->SetLabel(B_TRANSLATE("Chat settings"));

	fMembershipUpdates = new BCheckBox("MembershipUpdates",
		B_TRANSLATE("Show join/part messages"), new BMessage(kMembershipUpdates));

	fIgnoreEmoticons = new BCheckBox("IgnoreEmoticons",
		B_TRANSLATE("Ignore emoticons"), new BMessage(kIgnoreEmoticons));
	fIgnoreEmoticons->SetEnabled(false); // No emoticon support currently

	const float spacing = be_control_look->DefaultItemSpacing();


	BLayoutBuilder::Group<>(chatBox, B_VERTICAL)
		.SetInsets(spacing, spacing * 2, spacing, spacing)
		.Add(fMembershipUpdates)
		.Add(fIgnoreEmoticons)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(chatBox)
		.AddGlue()
	.End();
}


void
PreferencesChatWindow::AttachedToWindow()
{
	fIgnoreEmoticons->SetTarget(this);
	fIgnoreEmoticons->SetValue(AppPreferences::Get()->IgnoreEmoticons);
	fMembershipUpdates->SetTarget(this);
	fMembershipUpdates->SetValue(AppPreferences::Get()->MembershipUpdates);
}


void
PreferencesChatWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kIgnoreEmoticons:
			AppPreferences::Get()->IgnoreEmoticons = fIgnoreEmoticons->Value();
			break;
		case kMembershipUpdates:
			AppPreferences::Get()->MembershipUpdates = fMembershipUpdates->Value();
			break;
		default:
			BView::MessageReceived(message);
	}
}
