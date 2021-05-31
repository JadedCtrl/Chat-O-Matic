/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Dario Casalinuovo
 */
#include "User.h"

#include <libinterface/BitmapUtils.h>

#include "CayaProtocolAddOn.h"
#include "CayaResources.h"
#include "CayaUtils.h"
#include "Conversation.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
#include "UserItem.h"
#include "UserPopUp.h"


User::User(BString id, BMessenger msgn)
	:
	fID(id),
	fName(id),
	fMessenger(msgn),
	fLooper(NULL),
	fListItem(NULL),
	fStatus(CAYA_OFFLINE),
	fPopUp(NULL)
{
}


void
User::RegisterObserver(Conversation* chat)
{
	Notifier::RegisterObserver(chat);
	fConversations.AddItem(chat->GetId(), chat);
}


void
User::UnregisterObserver(Conversation* chat)
{
	Notifier::UnregisterObserver(chat);
	fConversations.RemoveItemFor(chat->GetId());
}


void
User::ShowPopUp(BPoint where)
{
	if (fPopUp == NULL) {
		fPopUp = new UserPopUp(this);
		RegisterObserver(fPopUp);
	}

	fPopUp->Show();
	fPopUp->MoveTo(where);
}


void
User::HidePopUp()
{
	if ((fPopUp != NULL) && !fPopUp->IsHidden())
		fPopUp->Hide();
}


void
User::DeletePopUp()
{
	if (fPopUp == NULL)
		return;

	if (fPopUp->Lock()) {
		UnregisterObserver(fPopUp);
		fPopUp->Quit();
		fPopUp = NULL;
	}
}


BString
User::GetId() const
{
	return fID;
}


BMessenger
User::Messenger() const
{
	return fMessenger;
}


void
User::SetMessenger(BMessenger messenger)
{
	fMessenger = messenger;
}


ProtocolLooper*
User::GetProtocolLooper() const
{
	return fLooper;
}


BString
User::GetName() const
{
	return fName;
}


BBitmap*
User::AvatarBitmap() const
{
	return fAvatarBitmap;
}


BBitmap*
User::ProtocolBitmap() const
{
	CayaProtocol* protocol = fLooper->Protocol();
	CayaProtocolAddOn* addOn
		= ProtocolManager::Get()->ProtocolAddOn(protocol->Signature());

	return addOn->ProtoIcon();
}


UserItem*
User::GetListItem()
{
	if (fListItem == NULL)
		fListItem = new UserItem(fName, this);
	return fListItem;
}


CayaStatus
User::GetNotifyStatus() const
{
	return fStatus;
}


BString
User::GetNotifyPersonalStatus() const
{
	return fPersonalStatus;
}


void
User::SetProtocolLooper(ProtocolLooper* looper)
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
User::SetNotifyName(BString name)
{
	if (fName.Compare(name) != 0) {
		fName = name;
		NotifyString(STR_CONTACT_NAME, name);
	}
}


void
User::SetNotifyAvatarBitmap(BBitmap* bitmap)
{
	if ((fAvatarBitmap != bitmap) && (bitmap != NULL)) {
		fAvatarBitmap = bitmap;
		NotifyPointer(PTR_AVATAR_BITMAP, (void*)bitmap);
	}
}


void
User::SetNotifyStatus(CayaStatus status)
{
	if (fStatus != status) {
		fStatus = status;
		NotifyInteger(INT_CONTACT_STATUS, (int32)fStatus);
	}
}


void
User::SetNotifyPersonalStatus(BString personalStatus)
{
	if (fPersonalStatus.Compare(personalStatus) != 0) {
		fPersonalStatus = personalStatus;
		NotifyString(STR_PERSONAL_STATUS, personalStatus);
	}
}


ChatMap
User::Conversations()
{
	return fConversations;
}


