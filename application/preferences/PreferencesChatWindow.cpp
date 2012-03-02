/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <CheckBox.h>
#include <ControlLook.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <StringView.h>

#include "CayaProtocol.h"
#include "PreferencesChatWindow.h"
#include "CayaPreferences.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "TheApp.h"

const uint32 kIgnoreEmoticons = 'CBhe';


PreferencesChatWindow::PreferencesChatWindow()
	: BView("Chat Window", B_WILL_DRAW)
{

	fChatWindowString = new BStringView("ChatWindowString", "Chat Window Settings");
	fChatWindowString->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fChatWindowString->SetFont(be_bold_font);

	fIgnoreEmoticons = new BCheckBox("IgnoreEmoticons",
		"Ignore Emoticons",
		new BMessage(kIgnoreEmoticons));
	fIgnoreEmoticons->SetEnabled(true);

	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_HORIZONTAL, spacing));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fChatWindowString)
		.AddGroup(B_VERTICAL, spacing)
			.Add(fIgnoreEmoticons)
			.SetInsets(spacing * 2, spacing, spacing, spacing)
		.End()
		.AddGlue()
		.SetInsets(spacing, spacing, spacing, spacing)
		.TopView()
	);
}


void
PreferencesChatWindow::AttachedToWindow()
{
	fIgnoreEmoticons->SetTarget(this);
	fIgnoreEmoticons->SetValue(
		CayaPreferences::Item()->IgnoreEmoticons);

}


void
PreferencesChatWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kIgnoreEmoticons:
			CayaPreferences::Item()->IgnoreEmoticons
				= fIgnoreEmoticons->Value();
			break;
		default:
			BView::MessageReceived(message);
	}
}
