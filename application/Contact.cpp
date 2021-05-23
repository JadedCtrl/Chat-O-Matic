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

#include "CayaPreferences.h"
#include "ChatWindow.h"
#include "RosterItem.h"
#include "WindowsManager.h"


Contact::Contact(BString id, BMessenger msgn)
	:
	User::User(id, msgn),
	fChatWindow(NULL),
	fNewWindow(true)
{
	fRosterItem = new RosterItem(id.String(), this);
	RegisterObserver(fRosterItem);
}


ChatWindow*
Contact::GetChatWindow()
{
	if (fChatWindow == NULL)
		_CreateChatWindow();
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
		_CreateChatWindow();

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


RosterItem*
Contact::GetRosterItem() const
{
	return fRosterItem;
}


void
Contact::SetNotifyAvatarBitmap(BBitmap* bitmap)
{
	User::SetNotifyAvatarBitmap(bitmap);
	if (fAvatarBitmap != NULL && fChatWindow != NULL)
		fChatWindow->UpdateAvatar();
}


void
Contact::_CreateChatWindow()
{
	fChatWindow = new ChatWindow(this);
	WindowsManager::Get()->RelocateWindow(fChatWindow);
	RegisterObserver(fChatWindow);
}
