/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_CONSTANTS_H
#define _CAYA_CONSTANTS_H

#include <GraphicsDefs.h>

/**
 * Color constants.
 */
const rgb_color CAYA_ORANGE_COLOR = {255, 186, 0, 255};
const rgb_color CAYA_GREEN_COLOR = {43, 134, 43, 255};
const rgb_color CAYA_RED_COLOR = {175, 1, 1, 255};
const rgb_color CAYA_WHITE_COLOR = {255, 255, 255, 255};
const rgb_color CAYA_BLACK_COLOR = {0, 0, 0, 255};
const rgb_color CAYA_SELSTART_COLOR = {254, 150, 57};
const rgb_color CAYA_SELEND_COLOR = {230, 113, 9};

/**
 * Miscellaneous.
 */
#define CAYA_UTF8_MUSICAL_NOTES "\xE2\x99\xAB"

/**
 * Status codes.
 */
enum CayaStatus {
	CAYA_ONLINE				= 1,
	CAYA_AWAY				= 2,
	CAYA_EXTENDED_AWAY		= 3,
	CAYA_DO_NOT_DISTURB		= 4,
	CAYA_OFFLINE			= 5,
	CAYA_STATUSES			= 6
};

#endif	// _CAYA_CONSTANTS_H
