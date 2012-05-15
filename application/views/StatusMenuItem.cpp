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

#include "CayaResources.h"
#include "CayaUtils.h"
#include "StatusMenuItem.h"

const float kSize	= 16;
const float kCircle	= 12;


StatusMenuItem::StatusMenuItem(const char* label, CayaStatus status,
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


CayaStatus
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
	BResources* res = CayaResources();
	if (!res)
		return;

	int32 num = 0;

	switch (fStatus) {
		case CAYA_ONLINE:
			num = kOnlineReplicant;
			break;
		case CAYA_EXTENDED_AWAY:
			num = kCayaIconReplicant;
			break;
		case CAYA_AWAY:
			num = kAwayReplicant;
			break;
		case CAYA_DO_NOT_DISTURB:
			num = kBusyReplicant;
			break;
		case CAYA_OFFLINE:
			num = kOfflineReplicant;
			break;
		default:
			break;
	}

	BBitmap* bitmap = IconFromResources(res, num, B_MINI_ICON);
	SetBitmap(bitmap);

	delete res;
}
