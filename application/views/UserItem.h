/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef USERITEM_H
#define USERITEM_H

#include <StringItem.h>

#include "Observer.h"

class User;


class UserItem : public BStringItem, public Observer {
public:
	UserItem(const char* name, User* user);

	User* GetUser();

protected:
	void ObserveString(int32 what, BString str);
	void ObservePointer(int32 what, void* ptr);
	void ObserveInteger(int32 what, int32 val);

private:
	User* fUser;
};


#endif // USERITEM_H

