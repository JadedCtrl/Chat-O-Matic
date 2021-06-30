/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _ROLE_H
#define _ROLE_H

#include <String.h>
#include <SupportDefs.h>

#include "Flags.h"


class Role {
public:
	Role()
		: fTitle("Default"), fPerms(0 | PERM_WRITE | PERM_READ), fPriority(0)
	{
	}

	Role(BString title, int32 perms, int32 priority)
		: fTitle(title), fPerms(perms), fPriority(priority)
	{
	}

	BString fTitle;
	int32 fPerms;		// Permissions afforded to role, as described above.
	int32 fPriority;	// 'Rank' of role, with higher being greater priority.
						// I.E., a user with a priority of 11 can't kick a user
						// with a priority of 12, but can one with 10.
						// This sort of hierarchy might not be universal in
						// chat protocols, but I think it can be adequately
						// simulated in add-ons.
};

#endif // _ROLE_H
