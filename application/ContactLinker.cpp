/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <stdio.h>

#include "CayaProtocolAddOn.h"
#include "ChatWindow.h"
#include "ContactLinker.h"
#include "ContactPopUp.h"
#include "NotifyMessage.h"
#include "ProtocolManager.h"
#include "RosterItem.h"
#include "WindowsManager.h"


ContactLinker::ContactLinker(BString id, BMessenger msgn)
	: fChatWindow(NULL),
	fID(id),
	fName(id),
	fMessenger(msgn),
	fLooper(NULL),
	fStatus(CAYA_OFFLINE),
	fPopUp(NULL)
{
	// Create the roster item and register it as observer
	fRosterItem = new RosterItem(id.String(), this);
	RegisterObserver(fRosterItem);

	// By default we use protocol icon as avatar icon
	CayaProtocolAddOn* addOn = ProtocolManager::Get()->ProtocolAddOn("gtalk");
	fAvatarBitmap = addOn->Icon();
}


ChatWindow*	
ContactLinker::GetChatWindow()
{
	if (fChatWindow == NULL)
		CreateChatWindow();
	return fChatWindow;
}


void 
ContactLinker::DeleteWindow()
{
	if (fChatWindow != NULL) {
		if (fChatWindow->Lock()) {
			UnregisterObserver(fChatWindow);
			fChatWindow->Quit();
			fChatWindow = NULL;
		}
	}
}


void
ContactLinker::ShowWindow()
{
	if (fChatWindow == NULL)
		CreateChatWindow();
	fChatWindow->SetWorkspaces(B_CURRENT_WORKSPACE);
	if (fChatWindow->IsHidden())
		fChatWindow->Show();
	fChatWindow->Activate(true);
}


void
ContactLinker::HideWindow()
{
	if ((fChatWindow != NULL) && !fChatWindow->IsHidden())
		fChatWindow->Hide();
}


void
ContactLinker::ShowPopUp(BPoint where)
{
	if (fPopUp == NULL) {
		fPopUp = new ContactPopUp(this);
		RegisterObserver(fPopUp);
	}

	fPopUp->Show();
	fPopUp->MoveTo(where);
}


void
ContactLinker::HidePopUp()
{
	if ((fPopUp != NULL) && !fPopUp->IsHidden())
		fPopUp->Hide();
}


void
ContactLinker::DeletePopUp()
{
	if (fPopUp == NULL)
		return;

	if (fPopUp->Lock()) {
		UnregisterObserver(fPopUp);
		fPopUp->Quit();
		fPopUp = NULL;
	}
}


BMessenger
ContactLinker::Messenger() const
{
	return fMessenger;
}


void
ContactLinker::SetMessenger(BMessenger messenger)
{
	fMessenger = messenger;
}


ProtocolLooper*
ContactLinker::GetProtocolLooper() const
{
	return fLooper;
}


void
ContactLinker::SetProtocolLooper(ProtocolLooper* looper)
{
	if (looper)
		fLooper = looper;
}


void
ContactLinker::SetNotifyName(BString name)
{	
	if (fName.Compare(name) != 0) {			
		fName = name;
		NotifyString(STR_CONTACT_NAME, name);
	}
}


void
ContactLinker::SetNotifyAvatarBitmap(BBitmap* bitmap)
{
	if ((fAvatarBitmap != bitmap) && (bitmap != NULL)) {
		fAvatarBitmap = bitmap;
		NotifyPointer(PTR_AVATAR_BITMAP, (void*)bitmap);
	}
}


void
ContactLinker::SetNotifyStatus(CayaStatus status)
{
	if (fStatus != status) {
		fStatus = status;
		NotifyInteger(INT_CONTACT_STATUS, (int32)fStatus);
	}
}


void
ContactLinker::SetNotifyPersonalStatus(BString personalStatus)
{
	if (fPersonalStatus.Compare(personalStatus) != 0) {
		fPersonalStatus = personalStatus;
		NotifyString(STR_PERSONAL_STATUS, personalStatus);
	}
}


void
ContactLinker::CreateChatWindow()
{
	fChatWindow = new ChatWindow(this);
	WindowsManager::Get()->RelocateWindow(fChatWindow);
	RegisterObserver(fChatWindow);
}
