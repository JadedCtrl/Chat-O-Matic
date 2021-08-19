/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "PreferencesNotifications.h"

#include <Box.h>
#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <Roster.h>

#include "AppPreferences.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesNotifications"


const uint32 kNotifyProtocolsLogin = 'NTpl';
const uint32 kNotifyContactStatus = 'NTcl';
const uint32 kNotifyNewMessage = 'NTms';
const uint32 kSoundOnMessageReceived = 'Fmsn';
const uint32 kSoundOnMention = 'FMsn';
const uint32 kEditSounds = 'HKsn';


PreferencesNotifications::PreferencesNotifications()
	: BView(B_TRANSLATE("Notifications"), B_WILL_DRAW)
{
	BBox* notificationBox = new BBox("notificationBox");
	notificationBox->SetLabel(B_TRANSLATE("Notifications"));

	fNotifyProtocols = new BCheckBox("EnableProtocolNotify",
		B_TRANSLATE("Enable protocol status notifications"),
		new BMessage(kNotifyProtocolsLogin));

	fNotifyContactStatus = new BCheckBox("EnableContactNotify",
		B_TRANSLATE("Enable contact status notifications"),
		new BMessage(kNotifyContactStatus));
	fNotifyContactStatus->SetEnabled(false); // not even this! dear god…
	
	fNotifyNewMessage = new BCheckBox("EnableMessageNotify",
		B_TRANSLATE("Enable message notifications"),
		new BMessage(kNotifyNewMessage));

	BBox* soundsBox = new BBox("soundsBox");
	soundsBox->SetLabel(B_TRANSLATE("Sounds"));

	fSoundOnMessageReceived = new BCheckBox("SoundOnMessageReceived",
		B_TRANSLATE("Sound on message received"),
		new BMessage(kSoundOnMessageReceived));

	fSoundOnMention = new BCheckBox("SoundOnMention",
		B_TRANSLATE("Sound when mentioned"),
		new BMessage(kSoundOnMention));

	fSoundsButton = new BButton("EditSoundsButton",
		B_TRANSLATE("Edit sounds" B_UTF8_ELLIPSIS), new BMessage(kEditSounds));

	const float spacing = be_control_look->DefaultItemSpacing();


	BLayoutBuilder::Group<>(notificationBox, B_VERTICAL)
		.SetInsets(spacing, spacing * 2, spacing, spacing)
		.Add(fNotifyProtocols)
		.Add(fNotifyContactStatus)
		.Add(fNotifyNewMessage)
	.End();

	BLayoutBuilder::Group<>(soundsBox, B_VERTICAL)
		.SetInsets(spacing, spacing * 2, spacing, spacing)
		.Add(fSoundOnMessageReceived)
		.Add(fSoundOnMention)
		.Add(fSoundsButton)
	.End();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(notificationBox)
		.Add(soundsBox)
		.AddGlue()
	.End();
}


void
PreferencesNotifications::AttachedToWindow()
{
	fNotifyProtocols->SetTarget(this);
	fNotifyContactStatus->SetTarget(this);
	fNotifyNewMessage->SetTarget(this);
	fSoundOnMessageReceived->SetTarget(this);
	fSoundOnMention->SetTarget(this);
	fSoundsButton->SetTarget(this);
	
	fNotifyProtocols->SetValue(
		AppPreferences::Get()->NotifyProtocolStatus);
	fNotifyContactStatus->SetValue(
		AppPreferences::Get()->NotifyContactStatus);
	fNotifyNewMessage->SetValue(
		AppPreferences::Get()->NotifyNewMessage);
	fSoundOnMessageReceived->SetValue(
		AppPreferences::Get()->SoundOnMessageReceived);
	fSoundOnMention->SetValue(
		AppPreferences::Get()->SoundOnMention);
}


void
PreferencesNotifications::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kNotifyProtocolsLogin:
			AppPreferences::Get()->NotifyProtocolStatus
				= fNotifyProtocols->Value();
			break;
		case kNotifyContactStatus:
			AppPreferences::Get()->NotifyContactStatus
				= fNotifyContactStatus->Value();
			break;
		case kNotifyNewMessage:
			AppPreferences::Get()->NotifyNewMessage
				= fNotifyNewMessage->Value();
			break;
		case kSoundOnMessageReceived:
			AppPreferences::Get()->SoundOnMessageReceived
				= fSoundOnMessageReceived->Value();
			break;
		case kSoundOnMention:
			AppPreferences::Get()->SoundOnMention
				= fSoundOnMention->Value();
			break;
		case kEditSounds:
			BRoster().Launch("application/x-vnd.haiku-Sounds");
			break;
		default:
			BView::MessageReceived(message);
	}
}
