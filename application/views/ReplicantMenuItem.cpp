/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2011, Pier Luigi Fiorini. All rights reserved.
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

#include "CayaMessages.h"
#include "CayaResources.h"
#include "CayaUtils.h"
#include "ReplicantStatusView.h"

const float kSize	= 16;
const float kCircle	= 12;


ReplicantMenuItem::ReplicantMenuItem(const char* label, CayaStatus status,
	bool custom, char shortcut, uint32 modifiers)
	:
	BitmapMenuItem(label, NULL, NULL, shortcut, modifiers),
	fStatus(status),
	fCustom(custom)
{
	BMessage* msg = new BMessage(CAYA_REPLICANT_STATUS_SET);
	msg->AddInt32("status", fStatus);
	SetMessage(msg);

	SetIcon();
}


CayaStatus
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
	BResources* res = CayaResources();
	if (!res)
		return;

	int32 num = 0;

	switch (fStatus) {
		case CAYA_ONLINE:
			num = kConnectingReplicant;
			break;
		case CAYA_EXTENDED_AWAY:
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
