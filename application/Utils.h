/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2014, Funky Idea Software
 * Copyright 2021, Jaidyn Levesque
 * Distributed under the terms of the MIT License.
 */
#ifndef _APP_UTILS_H
#define _APP_UTILS_H

#include <image.h>

#include <GraphicsDefs.h>
#include <Mime.h>
#include <Resources.h>

#include "AppConstants.h"
#include "UserStatus.h"

class BMenu;


// For display purposes
const char* UserStatusToString(UserStatus status);

// For use with the ImageCache
const char* UserStatusToImageKey(UserStatus status);

bool		IsCommand(BString line);
BString		CommandName(BString line);
BString		CommandArgs(BString line);

BResources ChatResources();

const char*	SettingsPath();

const char*	AccountsPath();
const char*	AccountPath(const char* signature, const char* subsignature);

BPath		CachePath();
BPath		AccountCachePath(const char* accountName);
BPath		RoomsCachePath(const char* accountName);
BPath		RoomsCachePath(BPath accPath);
BPath		RoomCachePath(const char* accountName, const char* roomIdentifier);
BPath		RoomCachePath(BPath accPath, const char* roomIdentifier);
BPath		UserCachePath(const char* accountName, const char* userIdentifier);
BPath		UserCachePath(BPath accPath, const char* userIdentifier);
BPath		ContactCachePath(const char* accountName, const char* userIdentifier);
BPath		ContactCachePath(BPath accPath, const char* userIdentifier);
BPath		AddOnCachePath(const char* signature);

rgb_color	TintColor(rgb_color color, int severity);
rgb_color	ForegroundColor(rgb_color background);

// Borrowed from BePodder's own libfunky. Groovy B)
status_t	ReadAttributeData(BNode* node, const char* name, char** buffer, int32 *size);
status_t	WriteAttributeMessage(BNode* node, const char* name, BMessage* data);
status_t	ReadAttributeMessage(BNode* node, const char* name, BMessage* data);

extern "C" status_t our_image(image_info& image);


#endif	// _APP_UTILS_H
