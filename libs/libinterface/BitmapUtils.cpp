/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Path.h>

#include <private/IconUtils.h>

#include "BitmapUtils.h"

#define BEOS_ICON_ATTRIBUTE			"BEOS:ICON"
#define BEOS_MINI_ICON_ATTRIBUTE 	"BEOS:M:STD_ICON"
#define BEOS_LARGE_ICON_ATTRIBUTE	"BEOS:L:STD_ICON"


BBitmap*
ReadNodeIcon(const char* name, icon_size size, bool followSymlink)
{
	BEntry entry(name, followSymlink);
	entry_ref ref;

	entry.GetRef(&ref);

	BNode node(BPath(&ref).Path());

	BBitmap* ret = new BBitmap(BRect(0, 0, (float)size - 1, (float)size - 1), B_RGBA32);
	if (BIconUtils::GetIcon(&node, BEOS_ICON_ATTRIBUTE, BEOS_MINI_ICON_ATTRIBUTE,
		BEOS_LARGE_ICON_ATTRIBUTE, size, ret) != B_OK) {
		delete ret;
		ret = NULL;
	}

	return ret;
}


BBitmap* IconFromResources(BResources* res, int32 num, icon_size size)
{
	if (!res)
		return NULL;

	size_t nbytes = 0;
	type_code type = B_VECTOR_ICON_TYPE;
	color_space cspace = B_RGBA32;

	// Try to find a vector icon
	const void* data = res->LoadResource(type, num, &nbytes);
	if (data == NULL) {
		// Determine resource type from icon size
		switch (size) {
			case B_MINI_ICON:
				type = B_MINI_ICON_TYPE;
				break;
			case B_LARGE_ICON:
				type = B_LARGE_ICON_TYPE;
				break;
			default:
				return NULL;
		}

		// Get bitmap icon
		data = res->LoadResource(type, num, &nbytes);
		if (data == NULL)
			return NULL;

		cspace = B_CMAP8;
	}

	BBitmap* icon = new BBitmap(BRect(0, 0, size - 1, size - 1), cspace);
	if (icon->InitCheck() != B_OK)
		return NULL;

	switch (type) {
		case B_VECTOR_ICON_TYPE:
			if (BIconUtils::GetVectorIcon((const uint8*)data, nbytes, icon) != B_OK) {
				delete icon;
				return NULL;
			}
			break;
		default:
			icon->SetBits(data, size * size, 0, cspace);
	}

	return icon;
}
