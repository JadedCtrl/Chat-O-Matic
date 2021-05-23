/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Dario Casalinuovo
 */
#include "Contact.h"

#include <libinterface/BitmapUtils.h>

#include "CayaUtils.h"
#include "CayaPreferences.h"
#include "CayaProtocolAddOn.h"
#include "CayaResources.h"
#include "ChatWindow.h"
#include "ContactPopUp.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
#include "RosterItem.h"
#include "WindowsManager.h"

#include <stdio.h>


Contact::Contact(BString id, BMessenger msgn)
	:
	fChatWindow(NULL),
	fID(id),
	fName(id),
	fMessenger(msgn),
	fLooper(NULL),
	fStatus(CAYA_OFFLINE),
	fPopUp(NULL),
	fNewWindow(true)
{
	// Create the roster item and register it as observer
	fRosterItem = new RosterItem(id.String(), this);
	RegisterObserver(fRosterItem);
}


ChatWindow*
Contact::GetChatWindow()
{
	if (fChatWindow == NULL)
		CreateChatWindow();
	return fChatWindow;
}


void
Contact::DeleteWindow()
{
	if (fChatWindow != NULL) {
		if (fChatWindow->Lock()) {
			UnregisterObserver(fChatWindow);
			fChatWindow->Quit();
			fChatWindow = NULL;
			fNewWindow = true;
		}
	}
}


void
Contact::ShowWindow(bool typing, bool userAction)
{
	if (fChatWindow == NULL)
		CreateChatWindow();

	fChatWindow->AvoidFocus(true);

	if (CayaPreferences::Item()->MoveToCurrentWorkspace)
		fChatWindow->SetWorkspaces(B_CURRENT_WORKSPACE);

	if (fNewWindow || userAction) {
		fChatWindow->AvoidFocus(false);
		fChatWindow->ShowWindow();
		fNewWindow = false;
	} else {
		if (typing) {
			if (CayaPreferences::Item()->RaiseUserIsTyping)
				fChatWindow->ShowWindow();
		} else {
			if (CayaPreferences::Item()->RaiseOnMessageReceived
			|| fChatWindow->IsHidden())
				fChatWindow->ShowWindow();
		}
	}
	fChatWindow->AvoidFocus(false);
}


void
Contact::HideWindow()
{
	if ((fChatWindow != NULL) && !fChatWindow->IsHidden())
		fChatWindow->Hide();
}


void
Contact::ShowPopUp(BPoint where)
{
	if (fPopUp == NULL) {
		fPopUp = new ContactPopUp(this);
		RegisterObserver(fPopUp);
	}

	fPopUp->Show();
	fPopUp->MoveTo(where);
}


void
Contact::HidePopUp()
{
	if ((fPopUp != NULL) && !fPopUp->IsHidden())
		fPopUp->Hide();
}


void
Contact::DeletePopUp()
{
	if (fPopUp == NULL)
		return;

	if (fPopUp->Lock()) {
		UnregisterObserver(fPopUp);
		fPopUp->Quit();
		fPopUp = NULL;
	}
}


RosterItem*
Contact::GetRosterItem() const
{
	return fRosterItem;
}


BString
Contact::GetId() const
{
	return fID;
}


BMessenger
Contact::Messenger() const
{
	return fMessenger;
}


void
Contact::SetMessenger(BMessenger messenger)
{
	fMessenger = messenger;
}


ProtocolLooper*
Contact::GetProtocolLooper() const
{
	return fLooper;
}


BString
Contact::GetName() const
{
	return fName;
}


BBitmap*
Contact::AvatarBitmap() const
{
	return fAvatarBitmap;
}


BBitmap*
Contact::ProtocolBitmap() const
{
	CayaProtocol* protocol = fLooper->Protocol();
	CayaProtocolAddOn* addOn
		= ProtocolManager::Get()->ProtocolAddOn(protocol->Signature());

	return addOn->ProtoIcon();
}


CayaStatus
Contact::GetNotifyStatus() const
{
	return fStatus;
}


BString
Contact::GetNotifyPersonalStatus() const
{
	return fPersonalStatus;
}


void
Contact::SetProtocolLooper(ProtocolLooper* looper)
{
	if (looper) {
		fLooper = looper;

		// By default we use the Person icon as avatar icon
		BResources* res = CayaResources();
		BBitmap* bitmap = IconFromResources(res,
			kPersonIcon, B_LARGE_ICON);

		SetNotifyAvatarBitmap(bitmap);
	}
}


void
Contact::SetNotifyName(BString name)
{
	if (fName.Compare(name) != 0) {
		fName = name;
		NotifyString(STR_CONTACT_NAME, name);
	}
}


void
Contact::SetNotifyAvatarBitmap(BBitmap* bitmap)
{
	if ((fAvatarBitmap != bitmap) && (bitmap != NULL)) {
		fAvatarBitmap = bitmap;
		NotifyPointer(PTR_AVATAR_BITMAP, (void*)bitmap);
		if (fChatWindow != NULL)
			fChatWindow->UpdateAvatar();
	}
}


void
Contact::SetNotifyStatus(CayaStatus status)
{
	if (fStatus != status) {
		fStatus = status;
		NotifyInteger(INT_CONTACT_STATUS, (int32)fStatus);
	}
}


void
Contact::SetNotifyPersonalStatus(BString personalStatus)
{
	if (fPersonalStatus.Compare(personalStatus) != 0) {
		fPersonalStatus = personalStatus;
		NotifyString(STR_PERSONAL_STATUS, personalStatus);
	}
}


void
Contact::CreateChatWindow()
{
	fChatWindow = new ChatWindow(this);
	WindowsManager::Get()->RelocateWindow(fChatWindow);
	RegisterObserver(fChatWindow);
}
