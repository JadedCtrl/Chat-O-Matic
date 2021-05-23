/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "UserPopUp.h"

#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <StringView.h>
#include <TextControl.h>

#include <libinterface/BitmapView.h>

#include "User.h"
#include "NotifyMessage.h"


const window_feel kMenuWindowFeel = window_feel(B_NORMAL_WINDOW_FEEL);

const int32 kNickChanged = 'NICH';


UserPopUp::UserPopUp(User* user)
	: BWindow(BRect(0, 0, 1, 1), "UserPopUp", B_BORDERED_WINDOW_LOOK,
		kMenuWindowFeel, B_NOT_MOVABLE | B_NOT_CLOSABLE | B_NOT_MINIMIZABLE |
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS |
		B_AVOID_FOCUS | B_AUTO_UPDATE_SIZE_LIMITS),
	fCoords(B_ORIGIN)
{
	// Box to change nick name
	fNickBox = new BTextControl("nickBox", NULL, user->GetName(),
		new BMessage(kNickChanged));

	// Real nick name
	fLabel = new BStringView("label", user->GetId());

	// Avatar bitmap
	fAvatarView = new BitmapView("avatarView");
	fAvatarView->SetBitmap(user->AvatarBitmap());

	// Layout
	SetLayout(new BGroupLayout(B_VERTICAL));
	AddChild(BGroupLayoutBuilder(B_HORIZONTAL, 10)
		.AddGroup(B_VERTICAL)
			.Add(fNickBox)
			.Add(fLabel)
			.AddGlue()
		.End()
		.Add(fAvatarView)
		.SetInsets(10, 10, 10, 10)
		.TopView()
	);
}


void
UserPopUp::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kNickChanged:
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


void
UserPopUp::MoveTo(BPoint where)
{
	if (fCoords != where) {
		if (Lock()) {
			BWindow::MoveTo(where);
			fCoords = where;
			Unlock();
		}
	}
}


void
UserPopUp::ObserveString(int32 what, BString str)
{
	switch (what) {
		case STR_CONTACT_NAME:
			if (Lock()) {
				fLabel->SetText(str);
				Unlock();
			}
			break;
	}
}


void
UserPopUp::ObservePointer(int32 what, void* ptr)
{
	switch (what) {
		case PTR_AVATAR_BITMAP:
			if (Lock()) {
				fAvatarView->SetBitmap((BBitmap*)ptr);
				Unlock();
			}
			break;
	}
}


void
UserPopUp::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_CONTACT_STATUS:
			break;
	}
}
