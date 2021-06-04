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
	void _UpdateColor(int32 status);
	rgb_color _TintColor(rgb_color color, int severity);

private:
	rgb_color fTextColor;
	User* fUser;
};


#endif // USERITEM_H

