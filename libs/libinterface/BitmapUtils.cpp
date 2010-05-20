/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <string.h>

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

	BBitmap* icon = new BBitmap(BRect(0, 0, (float)size - 1, (float)size - 1),
		cspace);
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


BBitmap*
RescaleBitmap(const BBitmap* src, int32 width, int32 height)
{
	width--; height--;

	if (!src || !src->IsValid())
		return NULL;

	BRect srcSize = src->Bounds();

	if (height < 0) {
		float srcProp = srcSize.Height() / srcSize.Width();
		height = (int32)(width * ceil(srcProp));
	}

	BBitmap* res = new BBitmap(BRect(0, 0, (float)width, (float)height),
		src->ColorSpace());

	float dx = (srcSize.Width() + 1) / (float)(width + 1);
	float dy = (srcSize.Height() + 1) / (float)(height + 1);
	uint8 bpp = (uint8)(src->BytesPerRow() / ceil(srcSize.Width()));

	int srcYOff = src->BytesPerRow();
	int dstYOff = res->BytesPerRow();

	void* dstData = res->Bits();
	void* srcData = src->Bits();

	for (int32 y = 0; y <= height; y++) {
		void* dstRow = (void*)((uint32)dstData + (uint32)(y * dstYOff));
		void* srcRow = (void*)((uint32)srcData + ((uint32)(y * dy)
			* srcYOff));

		for (int32 x = 0; x <= width; x++)
			memcpy((void*)((uint32)dstRow + (x * bpp)), (void*)((uint32)srcRow
				+ ((uint32)(x * dx) * bpp)), bpp);
	}

	return res;
}
