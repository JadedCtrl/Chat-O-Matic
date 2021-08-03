/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_VIEW_H
#define _STATUS_VIEW_H

#include <View.h>

#include "AppConstants.h"

class BPopUpMenu;

class BitmapView;
class MenuButton;
class NicknameTextControl;
class Server;

class StatusView : public BView {
public:
							StatusView(const char* name, Server* server);

	virtual	void			AttachedToWindow();
	virtual	void			MessageReceived(BMessage* msg);

			void			SetName(BString name);
			void			SetStatus(UserStatus status);
			void			SetAvatarIcon(const BBitmap* bitmap);

private:
			void			_PopulateAccountMenu();

	NicknameTextControl* 	fNickname;
	BitmapView*				fAvatar;
	BPopUpMenu*				fStatusMenu;

	MenuButton*				fAccountsButton;
	BPopUpMenu*				fAccountsMenu;

	Server*					fServer;
};

#endif	// _STATUS_VIEW_H
