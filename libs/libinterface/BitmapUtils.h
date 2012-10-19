/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _BITMAP_UTILS_H
#define _BITMAP_UTILS_H

#include <Bitmap.h>
#include <Mime.h>
#include <Resources.h>

BBitmap*	ReadNodeIcon(const char* name, icon_size size,
				bool followSymlink);
BBitmap*	IconFromResources(BResources* res, int32 num,
				icon_size size = B_LARGE_ICON);
BBitmap*	RescaleBitmap(const BBitmap* src, float width,
				float height);

#endif	// _BITMAP_UTILS_H
