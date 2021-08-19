/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _USER_STATUS_H
#define _USER_STATUS_H

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

#endif	// _USER_STATUS_H
