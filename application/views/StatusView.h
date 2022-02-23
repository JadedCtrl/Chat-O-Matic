/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_VIEW_H
#define _STATUS_VIEW_H

#include <View.h>

#include "UserStatus.h"
#include "Observer.h"

class BPopUpMenu;

class AccountsMenu;
class BitmapView;
class EnterTextView;
class MenuButton;

class StatusView : public BView, public Observer {
public:
							StatusView(const char* name);

	virtual	void			AttachedToWindow();
	virtual	void			MessageReceived(BMessage* msg);

	virtual void			ObserveString(int32 what, BString str);
	virtual void			ObserveInteger(int32 what, int32 value);
	virtual void			ObservePointer(int32 what, void* ptr);


private:
			void			_SetName(BString name);
			void			_SetStatus(UserStatus status);
			void			_SetAvatarIcon(const BBitmap* bitmap);

			void			_SetToAccount();

	EnterTextView*		 	fNickname;
	BitmapView*				fAvatar;
	BPopUpMenu*				fStatusMenu;

	MenuButton*				fAccountsButton;
	AccountsMenu*			fAccountsMenu;
	int64					fAccount;
};

#endif	// _STATUS_VIEW_H
