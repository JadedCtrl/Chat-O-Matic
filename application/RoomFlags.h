/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef ROOMFLAGS_H
#define ROOMFLAGS_H

#include <SupportDefs.h>

// AUTOJOIN, AUTOCREATE, LOG, POPULATE
// Auto-join on login, auto-create on login (non-persistent rooms), keep local
// logs, populate chat with local logs on join…

// JCLP
// 0000

#define ROOM_AUTOJOIN		1
#define ROOM_AUTOCREATE		2
#define ROOM_LOG_LOCALLY	4
#define ROOM_POPULATE_LOGS	8


#endif // ROOMFLAGS_H

