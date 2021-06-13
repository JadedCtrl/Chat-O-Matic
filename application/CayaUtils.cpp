/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2014, Funky Idea Software
 * Copyright 2021, Jaidyn Levesque
 * Distributed under the terms of the MIT License.
 */
#include <memory.h>

#include <Bitmap.h>
#include <InterfaceDefs.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <IconUtils.h>
#include <Path.h>

#include <kernel/fs_attr.h>

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


const char*
CayaCachePath()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;

	path.Append("Caya/Cache");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;

	return path.Path();
}


const char*
CayaAccountCachePath(const char* accountName)
{
	BPath path(CayaCachePath());
	if (path.InitCheck() != B_OK)
		return NULL;
	path.Append(accountName);

	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
CayaRoomsCachePath(const char* accountName)
{
	BPath path(CayaAccountCachePath(accountName));
	if (path.InitCheck() != B_OK)
		return NULL;
	path.Append("Rooms");

	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
CayaRoomCachePath(const char* accountName, const char* roomIdentifier)
{
	BPath path(CayaRoomsCachePath(accountName));
	if (path.InitCheck() != B_OK)
		return NULL;
	path.Append(roomIdentifier);
	return path.Path();
}


rgb_color
CayaTintColor(rgb_color color, int severity)
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
CayaForegroundColor(rgb_color background)
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
