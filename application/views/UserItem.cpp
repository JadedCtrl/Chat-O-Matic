/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UserItem.h"

#include "User.h"


UserItem::UserItem(const char* name, User* user)
	:
	BStringItem(name),
	fUser(user)
{
}


User*
UserItem::GetUser()
{
	return fUser;
}


void 
UserItem::ObserveString(int32 what, BString str)
{
}


void 
UserItem::ObservePointer(int32 what, void* ptr)
{
}


void 
UserItem::ObserveInteger(int32 what, int32 val)
{
}


