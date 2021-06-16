/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef DEFAULTITEMS_H
#define DEFAULTITEMS_H

#include "ChatCommand.h"

#include <ObjectList.h>


CommandMap				DefaultCommands();
BObjectList<BMessage>	DefaultUserPopUpItems();
BMessage*				_UserMenuItem(const char* label, BMessage* msg,
									  int32 user_perms, int32 target_perms,
									  int32 target_lacks, bool ignorePriority,
									  bool toProtocol);


#endif // DEFAULTITEMS_H
