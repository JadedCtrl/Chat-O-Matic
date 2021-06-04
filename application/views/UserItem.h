/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef USERITEM_H
#define USERITEM_H

#include <GraphicsDefs.h>
#include <StringItem.h>

#include "Observer.h"

class User;


class UserItem : public BStringItem, public Observer {
public:
	UserItem(const char* name, User* user, int32 status);

	void DrawItem(BView* owner, BRect frame, bool complete);

	void ObserveString(int32 what, BString str);
	void ObserveInteger(int32 what, int32 value);

	User* GetUser();

protected:
	rgb_color _GetTextColor(rgb_color highColor);

private:
	User* fUser;
	int fStatus;
};


#endif // USERITEM_H

