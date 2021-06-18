/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2014, Funky Idea Software
 * Copyright 2021, Jaidyn Levesque
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_UTILS_H
#define _CAYA_UTILS_H

#include <image.h>

#include <GraphicsDefs.h>
#include <Mime.h>
#include <Resources.h>

#include "CayaConstants.h"
#include "Server.h"

class BMenu;


const char* CayaStatusToString(CayaStatus status);

bool		IsCommand(BString line);
BString		CommandName(BString line);
BString		CommandArgs(BString line);

BResources* CayaResources();

BMenu* CreateAccountMenu(AccountInstances accounts, BMessage msg,
			BMessage* allMsg = NULL);

const char*	CayaAccountsPath();
const char*	CayaAccountPath(const char* signature);
const char*	CayaAccountPath(const char* signature, const char* subsignature);

const char* CayaCachePath();
const char* CayaAccountCachePath(const char* accountName);
const char* CayaRoomsCachePath(const char* accountName);
const char* CayaRoomCachePath(const char* accountName, const char* roomIdentifier);
const char* CayaUserCachePath(const char* accountName, const char* userIdentifier);
const char* CayaContactCachePath(const char* accountName, const char* userIdentifier);

rgb_color	CayaTintColor(rgb_color color, int severity);
rgb_color	CayaForegroundColor(rgb_color background);

// Borrowed from BePodder's own libfunky. Groovy B)
status_t	ReadAttributeData(BNode* node, const char* name, char** buffer, int32 *size);
status_t	WriteAttributeMessage(BNode* node, const char* name, BMessage* data);
status_t	ReadAttributeMessage(BNode* node, const char* name, BMessage* data);

extern "C" status_t our_image(image_info& image);


#endif	// _CAYA_UTILS_H

