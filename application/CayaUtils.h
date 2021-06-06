/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_UTILS_H
#define _CAYA_UTILS_H

#include <image.h>

#include <GraphicsDefs.h>
#include <Mime.h>
#include <Resources.h>

#include "CayaConstants.h"

const char* CayaStatusToString(CayaStatus status);

BResources* CayaResources();

const char*	CayaAccountsPath();
const char*	CayaAccountPath(const char* signature);
const char*	CayaAccountPath(const char* signature, const char* subsignature);

const char* CayaCachePath();
const char* CayaLogPath(const char* signature, const char* subsignature);

rgb_color	CayaTintColor(rgb_color color, int severity);

// Borrowed from BePodder's own libfunky
status_t	ReadAttributeData(BNode* node, const char* name, char** buffer, int32 *size);
status_t	WriteAttributeMessage(BNode* node, const char* name, BMessage* data);
status_t	ReadAttributeMessage(BNode* node, const char* name, BMessage* data);

extern "C" status_t our_image(image_info& image);


#endif	// _CAYA_UTILS_H

