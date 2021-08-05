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
					UserItem(User* user);
					~UserItem();

	virtual void	DrawItem(BView* owner, BRect frame, bool complete);

	virtual void	ObserveString(int32 what, BString str);
	virtual void	ObserveInteger(int32 what, int32 value);

			User*	GetUser();

protected:
		rgb_color	_GetTextColor(rgb_color highColor);

private:
	User* fUser;
	int fStatus;
};

#endif // USERITEM_H
