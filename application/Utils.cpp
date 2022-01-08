/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2014, Funky Idea Software
 * Copyright 2021, Jaidyn Levesque
 * Distributed under the terms of the MIT License.
 */
#include <memory.h>

#include <Bitmap.h>
#include <Catalog.h>
#include <InterfaceDefs.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <IconUtils.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Path.h>
#include <StringList.h>

#include <kernel/fs_attr.h>

#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Utils ― Status names"


const char*
UserStatusToString(UserStatus status)
{
	switch (status) {
		case STATUS_ONLINE:
			return B_TRANSLATE("Available");
		case STATUS_AWAY:
			return B_TRANSLATE("Away");
		case STATUS_DO_NOT_DISTURB:
			return B_TRANSLATE("Busy");
		case STATUS_CUSTOM_STATUS:
			return B_TRANSLATE("Custom Status");
		case STATUS_INVISIBLE:
			return B_TRANSLATE("Invisible");
		case STATUS_OFFLINE:
			return B_TRANSLATE("Offline");
		default:
			return NULL;
	}
}


const char*
UserStatusToImageKey(UserStatus status)
{
	switch (status) {
		case STATUS_ONLINE:
			return "kOnlineReplicant";
		case STATUS_AWAY:
			return "kAwayReplicant";
		case STATUS_OFFLINE:
			return "kOfflineReplicant";
		default:
			return "kBusyReplicant";
	}
}


bool
IsCommand(BString line)
{
	return line.StartsWith("/");
}


BString
CommandName(BString line)
{
	BStringList words;
	line.Split(" ", true, words);
	return words.StringAt(0).RemoveFirst("/");
}


BString
CommandArgs(BString line)
{
	BString remove("/");
	remove << CommandName(line) << "";
	line.RemoveFirst(remove);
	return line.Trim();
}


BResources
ChatResources()
{
	BResources res;
	image_info info;
	if (our_image(info) == B_OK) {
		BFile file(info.name, B_READ_ONLY);
		if (file.InitCheck() == B_OK)
			res.SetTo(&file);
	}
	return res;
}


const char*
SettingsPath()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;

	path.Append(APP_NAME);
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
AccountsPath()
{
	BPath path(SettingsPath());
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append("Accounts");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
AccountPath(const char* signature, const char* subsignature)
{
	BPath path(AccountsPath());
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append(signature);
	if (BString(signature) != BString(subsignature)
			&& BString(subsignature).IsEmpty() == false)
		path.Append(subsignature);

	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
CachePath()
{
	BPath path(SettingsPath());
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append("Cache");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
AccountCachePath(const char* accountName)
{
	BPath path(CachePath());
	path.Append("Accounts");
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append(accountName);
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
RoomsCachePath(const char* accountName)
{
	BPath path(AccountCachePath(accountName));
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append("Rooms");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
RoomCachePath(const char* accountName, const char* roomIdentifier)
{
	BPath path(RoomsCachePath(accountName));
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append(roomIdentifier);
	return path.Path();
}


const char*
UserCachePath(const char* accountName, const char* userIdentifier)
{
	BPath path(AccountCachePath(accountName));
	if (path.InitCheck() != B_OK)
		return NULL;

	path.Append("Users");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	path.Append(userIdentifier);
	return path.Path();
}


const char*
ContactCachePath(const char* accountName, const char* userIdentifier)
{
	BPath path(AccountCachePath(accountName));
	if (path.InitCheck() != B_OK)
		return NULL;
	path.Append("Contacts");

	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	path.Append(userIdentifier);
	return path.Path();
}


rgb_color
TintColor(rgb_color color, int severity)
{
	bool dark = false;
	if (color.Brightness() < 127)
		dark = true;

	switch (severity)
	{
		case 1:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_1_TINT + 0.2f);
			else
				return tint_color(color, B_DARKEN_1_TINT);
		case 2:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_1_TINT);
			else
				return tint_color(color, B_DARKEN_2_TINT);
		case 3:
			if (dark == true)
				return tint_color(color, B_LIGHTEN_2_TINT + 0.1f);
			else
				return tint_color(color, B_DARKEN_3_TINT);
	}
	return color;
}


rgb_color
ForegroundColor(rgb_color background)
{
	rgb_color foreground;
	int32 brighter;
	int32 darker;
	float ratio;

	do {
		foreground.set_to(rand() % 255, rand() % 255, rand() %255);
		if (foreground.Brightness() > background.Brightness()) {
			brighter = foreground.Brightness();
			darker = background.Brightness();
		}
		else {
			brighter = background.Brightness();
			darker = foreground.Brightness();
		}
		ratio = (brighter + .05) / (darker + .05);
	}
	while (ratio > 5 || ratio < 4);

	return foreground;
}


status_t
ReadAttributeData(BNode* node, const char* name, char** buffer, int32 *size) {
		attr_info info;
		status_t ret = node->GetAttrInfo(name, &info);

		if (ret == B_OK) {
			*buffer = (char *)calloc(info.size, sizeof(char));
			ret = node->ReadAttr(name, info.type, 0, *buffer, info.size);

			if (ret > B_OK) {
				*size = ret;
				ret = B_OK;
			}
			else
				free(*buffer);
		}

	return ret;
}


status_t
WriteAttributeMessage(BNode* node, const char* name, BMessage* data)
{
	BMallocIO	malloc;
	status_t ret=data->Flatten(&malloc);

	if(	ret == B_OK)	{
		ret = node->WriteAttr(name,B_ANY_TYPE,0,malloc.Buffer(),malloc.BufferLength());

		if(ret > B_OK)
			ret=B_OK;
	}

	return ret;
}


status_t
ReadAttributeMessage(BNode* node, const char* name, BMessage* data)
{
	char *buffer = NULL;
	int32 size = 0;

	status_t ret = ReadAttributeData(node,name,&buffer,&size);

	if(size>0 && buffer!=NULL) {
		BMemoryIO mem(buffer,size);
		ret = data->Unflatten(&mem);
		free(buffer);
	}

	return ret;
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
