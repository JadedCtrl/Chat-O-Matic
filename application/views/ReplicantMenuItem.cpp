/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2011-2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Dario Casalinuovo
 */
#include "ReplicantMenuItem.h"

#include <string.h>

#include <Bitmap.h>
#include <Menu.h>

#include <libinterface/BitmapUtils.h>

#include "AppMessages.h"
#include "AppResources.h"
#include "ReplicantStatusView.h"
#include "Utils.h"

const float kSize	= 16;
const float kCircle	= 12;


ReplicantMenuItem::ReplicantMenuItem(const char* label, UserStatus status,
	bool custom, char shortcut, uint32 modifiers)
	:
	BitmapMenuItem(label, NULL, NULL, shortcut, modifiers),
	fStatus(status),
	fCustom(custom)
{
	BMessage* msg = new BMessage(APP_REPLICANT_STATUS_SET);
	msg->AddInt32("status", fStatus);
	SetMessage(msg);

	SetIcon();
}


UserStatus
ReplicantMenuItem::Status() const
{
	return fStatus;
}


bool
ReplicantMenuItem::IsCustom() const
{
	return fCustom;
}


void
ReplicantMenuItem::SetIcon()
{
	BResources res = ChatResources();
	if (res.InitCheck() != B_OK)
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

	BBitmap* bitmap = IconFromResources(&res, num, B_MINI_ICON);
	SetBitmap(bitmap);
}
