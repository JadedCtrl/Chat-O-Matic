/*
 * Copyright 2022, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _MAPS_H
#define _MAPS_H

#include <String.h>

#include "libsupport/KeyMap.h"

class ChatCommand;
class Command;
class Contact;
class Conversation;
class User;


// Defining some commonly-used KeyMaps
typedef KeyMap<BString, bigtime_t> AccountInstances;
typedef KeyMap<BString, ChatCommand*> CommandMap;
typedef KeyMap<BString, Conversation*> ChatMap;
typedef KeyMap<BString, Contact*> RosterMap;
typedef KeyMap<BString, User*> UserMap;

#endif // _MAPS_H
