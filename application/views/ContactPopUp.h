/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_POPUP_H
#define _CONTACT_POPUP_H

#include <Window.h>

#include "Observer.h"

class BTextControl;
class BStringView;

class BitmapView;
class ContactLinker;

class ContactPopUp : public BWindow, public Observer {
public:
						ContactPopUp(ContactLinker* contact);

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

#endif	// _CONTACT_POPUP_H
