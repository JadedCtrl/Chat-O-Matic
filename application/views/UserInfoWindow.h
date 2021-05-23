/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Casalinuovo Dario
 */
#ifndef _USER_INFO_WINDOW_H
#define _USER_INFO_WINDOW_H

#include <Window.h>
#include <TextView.h>
#include <StringView.h>
#include "Observer.h"

#include "CayaConstants.h"

class BitmapView;
class User;


class UserInfoWindow: public BWindow, public Observer {
public:
						UserInfoWindow(User* user);

	virtual	void		MessageReceived(BMessage* message);

private:
		BTextView*		fStatus;
		User*			fUser;
		BTextView*		fPersonalMessage;
		BitmapView*		fAvatar;
};


#endif	// _USER_INFO_WINDOW_H

