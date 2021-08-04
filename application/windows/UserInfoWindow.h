/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Casalinuovo Dario
 *		Jaidyn Levesque <jadedctrl@teknik.io>
 */
#ifndef _USER_INFO_WINDOW_H
#define _USER_INFO_WINDOW_H

#include <Window.h>
#include <TextView.h>
#include <StringView.h>

#include "AppConstants.h"
#include "Observer.h"

class BitmapView;
class User;


class UserInfoWindow: public BWindow, public Observer {
public:
						UserInfoWindow(User* user);
						~UserInfoWindow();

	virtual	void		MessageReceived(BMessage* message);

	virtual void		ObserveString(int32 what, BString string);
	virtual void		ObserveInteger(int32 what, int32 num);
	virtual void		ObservePointer(int32 what, void* ptr);

private:
			void		_InitInterface();

			void		_UpdateStatusViews(UserStatus status);

		User*			fUser;

		BitmapView*		fAvatar;
		BStringView*	fNameLabel;
		BTextView*		fIdLabel;
		BitmapView*		fStatusIcon;
		BStringView*	fStatusLabel;
		BStringView*	fTextStatusLabel;
};

#endif	// _USER_INFO_WINDOW_H
