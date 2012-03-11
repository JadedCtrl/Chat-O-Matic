/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Casalinuovo Dario
 */
#ifndef _CONTACT_INFO_WINDOW_H
#define _CONTACT_INFO_WINDOW_H

#include <Window.h>
#include <TextView.h>
#include <StringView.h>
#include "Observer.h"

#include "CayaConstants.h"

class BitmapView;
class ContactLinker;

class ContactInfoWindow: public BWindow, public Observer {
public:
						ContactInfoWindow(ContactLinker* linker);

	virtual	void		MessageReceived(BMessage* message);
private:
		BTextView*	fStatus;
		ContactLinker*	fContactLinker;
		BTextView*		fPersonalMessage;
		BitmapView*		fAvatar;

};

#endif
