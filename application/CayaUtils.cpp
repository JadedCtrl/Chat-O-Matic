/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <memory.h>

#include <Bitmap.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <IconUtils.h>
#include <Path.h>

#include "CayaUtils.h"


const char*
CayaStatusToString(CayaStatus status)
{
	switch (status) {
		case CAYA_ONLINE:
			return "Available";
		case CAYA_AWAY:
			return "Away";
		case CAYA_DO_NOT_DISTURB:
			return "Busy";
		case CAYA_CUSTOM_STATUS:
			return "Custom Status";
		case CAYA_INVISIBLE:
			return "Invisible";
		case CAYA_OFFLINE:
			return "Offline";
		default:
			return NULL;
	}
}


BResources*
CayaResources()
{
	image_info info;
	if (our_image(info) != B_OK)
		return NULL;

	BFile file(info.name, B_READ_ONLY);
	if (file.InitCheck() != B_OK)
		return NULL;

	BResources* res = new BResources(&file);
	if (res->InitCheck() != B_OK) {
		delete res;
		return NULL;
	}

	return res;
}


const char*
CayaAccountsPath()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;

	path.Append("Caya/Protocols");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;

	return path.Path();
}


const char*
CayaAccountPath(const char* signature)
{
	if (!signature)
		return NULL;

	BPath path(CayaAccountsPath());
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append(signature);
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;

	return path.Path();
}


const char*
CayaAccountPath(const char* signature, const char* subsignature)
{
	if (BString(signature) == BString(subsignature)
		|| BString(subsignature).IsEmpty() == true)
		return CayaAccountPath(signature);

	BPath path(CayaAccountPath(signature));

	path.Append(subsignature);
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;

	return path.Path();
}


extern "C" {

status_t
our_image(image_info& image)
{
	team_id team = B_CURRENT_TEAM;

	int32 cookie = 0;
	while (get_next_image_info(team, &cookie, &image) == B_OK) {
		if ((char*)our_image >= (char*)image.text
			&& (char*)our_image <= (char*)image.text + image.text_size)
			return B_OK;
	}

	return B_ERROR;
}

}
