/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_VIEW_H
#define _STATUS_VIEW_H

#include <View.h>

#include "CayaConstants.h"

class BPopUpMenu;

class BitmapView;
class NicknameTextControl;

class StatusView : public BView {
public:
							StatusView(const char* name);

	virtual	void			AttachedToWindow();
	virtual	void			MessageReceived(BMessage* msg);

			void			SetName(BString name);
			void			SetStatus(CayaStatus status);
			void			SetAvatar(BBitmap* bitmap);

private:
	BPopUpMenu*				fStatusMenu;
	NicknameTextControl* 	fNickname;
	BitmapView*				fAvatar;
};

#endif	// _STATUS_VIEW_H
