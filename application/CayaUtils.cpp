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


BBitmap*
RescaleBitmap(const BBitmap* src, int32 width, int32 height)
{
	width--; height--;

	if (!src || !src->IsValid())
		return NULL;

	BRect srcSize = src->Bounds();

	if (height < 0) {
		float srcProp = srcSize.Height() / srcSize.Width();
		height = (int32)(width * srcProp);
	}

	BBitmap* res = new BBitmap(BRect(0, 0, width, height), src->ColorSpace());

	float dx = (srcSize.Width() + 1) / (width + 1);
	float dy = (srcSize.Height() + 1) / (height + 1);
	uint8 bpp = (uint8)(src->BytesPerRow() / srcSize.Width());

	int srcYOff = src->BytesPerRow();
	int dstYOff = res->BytesPerRow();

	void* dstData = res->Bits();
	void* srcData = src->Bits();

	for (int32 y = 0; y <= height; y++) {
		void* dstRow = (void *)((uint32)dstData + (uint32)(y * dstYOff));
		void* srcRow = (void *)((uint32)srcData + ((uint32)(y * dy) * srcYOff));

		for (int32 x = 0; x <= width; x++)
			memcpy((void*)((uint32)dstRow + (x * bpp)), (void*)((uint32)srcRow +
				((uint32)(x * dx) * bpp)), bpp);
	}

	return res;
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
