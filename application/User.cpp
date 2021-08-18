/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Dario Casalinuovo
 */
#include "User.h"

#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslationUtils.h>
#include <TranslatorRoster.h>

#include "ChatProtocolAddOn.h"
#include "AppResources.h"
#include "Conversation.h"
#include "ImageCache.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
#include "UserPopUp.h"
#include "Utils.h"


User::User(BString id, BMessenger msgn)
	:
	fID(id),
	fName(id),
	fMessenger(msgn),
	fLooper(NULL),
	fItemColor(ForegroundColor(ui_color(B_LIST_BACKGROUND_COLOR))),
	fStatus(STATUS_ONLINE),
	fAvatarBitmap(NULL),
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
	if (fAvatarBitmap == NULL)
		return ImageCache::Get()->GetImage("kPersonIcon");
	return fAvatarBitmap;
}


BBitmap*
User::ProtocolBitmap() const
{
	ChatProtocol* protocol = fLooper->Protocol();
	ChatProtocolAddOn* addOn
		= ProtocolManager::Get()->ProtocolAddOn(protocol->Signature());

	return addOn->ProtoIcon();
}


UserStatus
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
	if (looper != NULL) {
		fLooper = looper;
		BBitmap* avatar = _GetCachedAvatar();
		if (avatar != NULL && avatar->IsValid()) {
			fAvatarBitmap = avatar;
			NotifyPointer(PTR_AVATAR_BITMAP, (void*)avatar);
		}
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
		_SetCachedAvatar(bitmap);
		NotifyPointer(PTR_AVATAR_BITMAP, (void*)_GetCachedAvatar());
	}
}


void
User::SetNotifyStatus(UserStatus status)
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


void
User::_EnsureCachePath()
{
	if (fCachePath.InitCheck() == B_OK)
		return;
	fCachePath.SetTo(UserCachePath(fLooper->Protocol()->GetName(),
									   fID.String()));
}


BBitmap*
User::_GetCachedAvatar()
{
	_EnsureCachePath();

	// Try loading cached avatar
	BFile cacheFile(fCachePath.Path(), B_READ_ONLY);
	BBitmap* bitmap = BTranslationUtils::GetBitmap(&cacheFile);
	if (bitmap != NULL && bitmap->IsValid() == true)
		return bitmap;
	return NULL;
}


void
User::_SetCachedAvatar(BBitmap* bitmap)
{
	_EnsureCachePath();
	BFile cacheFile(fCachePath.Path(), B_WRITE_ONLY | B_CREATE_FILE);
	if (cacheFile.InitCheck() != B_OK)
		return;

	BBitmapStream stream(bitmap);
	BTranslatorRoster* roster = BTranslatorRoster::Default();

	int32 format_count;
	translator_info info;
	const translation_format* formats = NULL;
	roster->Identify(&stream, new BMessage(), &info, 0, "image");
	roster->GetOutputFormats(info.translator, &formats, &format_count);

	roster->Translate(info.translator, &stream, NULL, &cacheFile,
		formats[0].type);
}
