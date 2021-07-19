/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <Button.h>
#include <Catalog.h>
#include <CheckBox.h>
#include <ControlLook.h>
#include <Deskbar.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ScrollView.h>
#include <StringView.h>

#include "AccountManager.h"
#include "ChatProtocol.h"
#include "PreferencesBehavior.h"
#include "AppPreferences.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "ReplicantStatusView.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesBehavior"


const uint32 kToCurrentWorkspace = 'CBcw';
const uint32 kRaiseOnMessageReceived = 'FCmr';
const uint32 kRaiseUserIsTyping = 'FCit';
const uint32 kNotifyProtocolsLogin = 'NTpl';
const uint32 kNotifyContactStatus = 'NTcl';
const uint32 kNotifyNewMessage = 'NTms';
const uint32 kMarkUnreadWindow = 'MKuw';
const uint32 kHideOffline = 'HiOf';
const uint32 kDisablePrompt = 'DiPr';


PreferencesBehavior::PreferencesBehavior()
	: BView(B_TRANSLATE("Behavior"), B_WILL_DRAW)
{

	fOnIncoming = new BStringView("onIncoming", B_TRANSLATE("On incoming "
		"message" B_UTF8_ELLIPSIS));
	fOnIncoming->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fOnIncoming->SetFont(be_bold_font);
	
	fHideOffline = new BCheckBox("HideOfflineContacts",
		B_TRANSLATE("Hide offline contacts"),
		new BMessage(kHideOffline));

	fToCurrentWorkspace = new BCheckBox("ToCurrentWorkspace",
		B_TRANSLATE("Move window to current workspace"),
		new BMessage(kToCurrentWorkspace));

	fRaiseOnMessageReceived = new BCheckBox("FocusOnMessageReceived",
		B_TRANSLATE("Auto-raise when a message is received"),
		new BMessage(kRaiseOnMessageReceived));

	fRaiseUserIsTyping = new BCheckBox("FocusUserIsTyping",
		B_TRANSLATE("Auto-raise when user is typing"),
		new BMessage(kRaiseUserIsTyping));

	fPlaySoundOnMessageReceived = new BCheckBox("PlaySoundOnMessageReceived",
		B_TRANSLATE("Play sound event"), NULL);
	fPlaySoundOnMessageReceived->SetEnabled(false);  // not implemented

	fMarkUnreadWindow = new BCheckBox("MarkUnreadWindow",
		B_TRANSLATE("Mark unread window chat"),
		new BMessage(kMarkUnreadWindow));
	/*fMarkUnreadWindow->SetEnabled(false); implementing it right now*/
	
	fMarkUnreadReplicant = new BCheckBox("MarkUnreadReplicant",
		B_TRANSLATE("Mark unread the Deskbar Replicant"), NULL);
	fMarkUnreadReplicant->SetEnabled(false);
			// not implemented

	fNotifications = new BStringView("notifications",
						B_TRANSLATE("Deskbar Notifications (experimental)"));

	fNotifications->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fNotifications->SetFont(be_bold_font);

	fNotifyProtocols = new BCheckBox("EnableProtocolNotify",
		B_TRANSLATE("Enable protocol status notifications"),
		new BMessage(kNotifyProtocolsLogin));

	fNotifyContactStatus = new BCheckBox("EnableContactNotify",
		B_TRANSLATE("Enable contact status notifications"),
		new BMessage(kNotifyContactStatus));
	
	fNotifyNewMessage = new BCheckBox("EnableMessageNotify",
		B_TRANSLATE("Enable message notifications"),
		new BMessage(kNotifyNewMessage));

	fGeneral = new BStringView("onGeneral", B_TRANSLATE("General"));
	fGeneral->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));
	fGeneral->SetFont(be_bold_font);

	fDisableQuitConfirm = new BCheckBox("DisableQuitConfirm",
		B_TRANSLATE("Don't ask confirmation at Quit"),
		new BMessage(kDisablePrompt));
	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_HORIZONTAL, spacing));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(fOnIncoming)
		.AddGroup(B_VERTICAL, spacing)
			.Add(fHideOffline)
			.Add(fToCurrentWorkspace)
			.Add(fRaiseOnMessageReceived)
			.Add(fRaiseUserIsTyping)
			.Add(fMarkUnreadWindow)
			.Add(fMarkUnreadReplicant)
			.Add(fPlaySoundOnMessageReceived)
		.	SetInsets(spacing * 2, spacing, spacing, spacing)
		.End()
		.Add(fGeneral)
		.AddGroup(B_VERTICAL, spacing)
			.Add(fDisableQuitConfirm)
		.	SetInsets(spacing * 2, spacing, spacing, spacing)
		.End()
		.Add(fNotifications)
		.AddGroup(B_VERTICAL, spacing)
			.Add(fNotifyProtocols)
			.Add(fNotifyContactStatus)
			.Add(fNotifyNewMessage)
		.	SetInsets(spacing * 2, spacing, spacing, spacing)
		.End()
		.AddGlue()
		.SetInsets(spacing, spacing, spacing, spacing)
		.TopView()
	);
}


void
PreferencesBehavior::AttachedToWindow()
{
	fHideOffline->SetTarget(this);
	fToCurrentWorkspace->SetTarget(this);
	fRaiseUserIsTyping->SetTarget(this);
	fRaiseOnMessageReceived->SetTarget(this);
	fNotifyProtocols->SetTarget(this);
	fNotifyContactStatus->SetTarget(this);
	fNotifyNewMessage->SetTarget(this);
	fDisableQuitConfirm->SetTarget(this);
	
	fHideOffline->SetValue(
		AppPreferences::Item()->HideOffline);
	fToCurrentWorkspace->SetValue(
		AppPreferences::Item()->MoveToCurrentWorkspace);
	fRaiseUserIsTyping->SetValue(
		AppPreferences::Item()->RaiseUserIsTyping);
	fRaiseOnMessageReceived->SetValue(
		AppPreferences::Item()->RaiseOnMessageReceived);
	fMarkUnreadWindow->SetValue(
		AppPreferences::Item()->MarkUnreadWindow);
	fNotifyProtocols->SetValue(
		AppPreferences::Item()->NotifyProtocolStatus);
	fNotifyContactStatus->SetValue(
		AppPreferences::Item()->NotifyContactStatus);
	fNotifyNewMessage->SetValue(
		AppPreferences::Item()->NotifyNewMessage);
	fDisableQuitConfirm->SetValue(
		AppPreferences::Item()->DisableQuitConfirm);
}


void
PreferencesBehavior::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kHideOffline:
			AppPreferences::Item()->HideOffline
				= fHideOffline->Value();
			break;
		case kToCurrentWorkspace:
			AppPreferences::Item()->MoveToCurrentWorkspace
				= fToCurrentWorkspace->Value();
			break;
		case kRaiseOnMessageReceived:
			AppPreferences::Item()->RaiseOnMessageReceived
				= fRaiseOnMessageReceived->Value();
			break;
		case kRaiseUserIsTyping:
			AppPreferences::Item()->RaiseUserIsTyping
				= fRaiseUserIsTyping->Value();
			break;
		case kNotifyProtocolsLogin:
			AppPreferences::Item()->NotifyProtocolStatus
				= fNotifyProtocols->Value();
			break;
		case kNotifyContactStatus:
			AppPreferences::Item()->NotifyContactStatus
				= fNotifyContactStatus->Value();
			break;
		case kNotifyNewMessage:
			AppPreferences::Item()->NotifyNewMessage
				= fNotifyNewMessage->Value();
			break;
		case kMarkUnreadWindow:
			AppPreferences::Item()->MarkUnreadWindow
				= fMarkUnreadWindow->Value();
			break;
		case kDisablePrompt:
			AppPreferences::Item()->DisableQuitConfirm
				= fDisableQuitConfirm->Value();
			break;
		default:
			BView::MessageReceived(message);
	}
}
