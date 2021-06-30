/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef FLAGS_H
#define FLAGS_H

// AUTOJOIN, AUTOCREATE, LOG, POPULATE
// Auto-join on login, auto-create on login (non-persistent rooms), keep local
// logs, populate chat with local logs on join…

// JCLP
// 0000

#define ROOM_AUTOJOIN		1
#define ROOM_AUTOCREATE		2
#define ROOM_LOG_LOCALLY	4
#define ROOM_POPULATE_LOGS	8


// NAME, SUBJECT, ROLECHANGE, BAN, KICK, DEAFEN, MUTE, NICK, READ, WRITE
// Set name of room, set subject, change user's "role" (permission presets
// defined by the protocol), etc…

// NSRBKDMNRW
// 0000000000

#define PERM_WRITE			1
#define PERM_READ			2
#define PERM_NICK			4
#define PERM_MUTE			8
#define PERM_DEAFEN			16
#define PERM_KICK			32
#define PERM_BAN			64
#define PERM_ROLECHANGE		128
#define PERM_ROOM_SUBJECT	256
#define PERM_ROOM_NAME		512
#define PERM_ALL			1023

#endif // FLAGS_H
