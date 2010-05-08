/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <memory.h>

#include <Bitmap.h>

#include "private/IconUtils.h"

#include "CayaUtils.h"


const char*
CayaStatusToString(CayaStatus status)
{
	switch (status) {
		case CAYA_ONLINE:
			return "Available";
		case CAYA_OFFLINE:
			return "Offline";
		case CAYA_AWAY:
			return "Away";
		case CAYA_EXTENDED_AWAY:
			return "Extended Away";
		case CAYA_DO_NOT_DISTURB:
			return "Busy";
		default:
			return NULL;
	}
}

extern "C" {

status_t
our_image(image_info& image)
{
	team_id team = B_CURRENT_TEAM;

	int32 cookie = 0;
	while (get_next_image_info(team, &cookie, &image) == B_OK) {
		if ((char *)our_image >= (char *)image.text
			&& (char *)our_image <= (char *)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}

}
