/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_UTILS_H
#define _CAYA_UTILS_H

#include <image.h>
#include <Mime.h>

#include "CayaConstants.h"

class BBitmap;
class BResources;

const char* CayaStatusToString(CayaStatus status);

BBitmap* RescaleBitmap(const BBitmap* src, int32 width, int32 height);

extern "C" status_t our_image(image_info& image);

#endif	// _CAYA_UTILS_H
