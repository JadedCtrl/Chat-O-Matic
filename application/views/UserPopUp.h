/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _USER_POPUP_H
#define _USER_POPUP_H

#include <Window.h>

#include "Observer.h"

class BTextControl;
class BStringView;

class BitmapView;
class User;

class UserPopUp : public BWindow, public Observer {
public:
						UserPopUp(User* user);

	virtual	void		MessageReceived(BMessage* msg);

			void		MoveTo(BPoint where);

protected:
			void		ObserveString(int32 what, BString str);
			void		ObservePointer(int32 what, void* ptr);
			void		ObserveInteger(int32 what, int32 val);

private:
	BPoint				fCoords;
	BTextControl*		fNickBox;
	BStringView*		fLabel;
	BitmapView*			fAvatarView;
};


#endif	// _USER_POPUP_H

