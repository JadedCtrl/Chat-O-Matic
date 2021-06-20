/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <string.h>

#include <Bitmap.h>
#include <Menu.h>

#include <libinterface/BitmapUtils.h>

#include "AppResources.h"
#include "StatusMenuItem.h"
#include "Utils.h"

const float kSize	= 16;
const float kCircle	= 12;


StatusMenuItem::StatusMenuItem(const char* label, UserStatus status,
	bool custom, char shortcut, uint32 modifiers)
	:
	BitmapMenuItem(label, NULL, NULL, shortcut, modifiers),
	fStatus(status),
	fCustom(custom)
{
	BMessage* msg = new BMessage(kSetStatus);
	msg->AddInt32("status", fStatus);
	msg->AddBool("custom", fCustom);
	SetMessage(msg);

	SetIcon();
}


UserStatus
StatusMenuItem::Status() const
{
	return fStatus;
}


bool
StatusMenuItem::IsCustom() const
{
	return fCustom;
}


void
StatusMenuItem::SetIcon()
{
	BResources* res = ChatResources();
	if (!res)
		return;

	int32 num = 0;

	switch (fStatus) {
		case STATUS_ONLINE:
			num = kOnlineReplicant;
			break;
		case STATUS_AWAY:
			num = kAwayReplicant;
			break;
		case STATUS_DO_NOT_DISTURB:
			num = kBusyReplicant;
			break;
		case STATUS_CUSTOM_STATUS:
			num = kIconReplicant;
			break;
		case STATUS_INVISIBLE:
		case STATUS_OFFLINE:
			num = kOfflineReplicant;
			break;
		default:
			break;
	}

	BBitmap* bitmap = IconFromResources(res, num, B_MINI_ICON);
	SetBitmap(bitmap);

	delete res;
}
