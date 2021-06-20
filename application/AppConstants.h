/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _APP_CONSTANTS_H
#define _APP_CONSTANTS_H

#include <GraphicsDefs.h>

/**
 * Color constants.
 */
const rgb_color APP_ORANGE_COLOR = {255, 186, 0, 255};
const rgb_color APP_GREEN_COLOR = {43, 134, 43, 255};
const rgb_color APP_RED_COLOR = {175, 1, 1, 255};
const rgb_color APP_WHITE_COLOR = {255, 255, 255, 255};
const rgb_color APP_BLACK_COLOR = {0, 0, 0, 255};
const rgb_color APP_SELSTART_COLOR = {254, 150, 57};
const rgb_color APP_SELEND_COLOR = {230, 113, 9};

/**
 * Miscellaneous.
 */
#define APP_UTF8_MUSICAL_NOTES "\xE2\x99\xAB"

/**
 * Status codes.
 */
enum UserStatus {
	STATUS_ONLINE			= 1,
	STATUS_AWAY				= 2,
	STATUS_DO_NOT_DISTURB	= 3,
 	STATUS_CUSTOM_STATUS	= 4,
 	STATUS_INVISIBLE		= 5,
	STATUS_OFFLINE			= 6,
	STATUS_STATUSES			= 7
};

#endif	// _APP_CONSTANTS_H
