/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <StringView.h>
#include <TextControl.h>

#include "BitmapView.h"
#include "ContactLinker.h"
#include "ContactPopUp.h"
#include "NotifyMessage.h"

const window_feel kMenuWindowFeel = window_feel(1025);

const int32 kNickChanged = 'NICH';


ContactPopUp::ContactPopUp(ContactLinker* contact)
	: BWindow(BRect(0, 0, 1, 1), "ContactPopUp", B_BORDERED_WINDOW_LOOK,
		kMenuWindowFeel, B_NOT_MOVABLE | B_NOT_CLOSABLE | B_NOT_MINIMIZABLE |
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE | B_ASYNCHRONOUS_CONTROLS |
		B_AVOID_FOCUS | B_AUTO_UPDATE_SIZE_LIMITS),
	fCoords(B_ORIGIN)
{
	// Box to change nick name
	fNickBox = new BTextControl("nickBox", NULL, contact->GetName(),
		new BMessage(kNickChanged));

	// Real nick name
	fLabel = new BStringView("label", contact->GetId());

	// Avatar bitmap
	fAvatarView = new BitmapView("avatarView");
	fAvatarView->SetBitmap(contact->AvatarBitmap());

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
ContactPopUp::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kNickChanged:
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}


void
ContactPopUp::MoveTo(BPoint where)
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
ContactPopUp::ObserveString(int32 what, BString str)
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
ContactPopUp::ObservePointer(int32 what, void* ptr)
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
ContactPopUp::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_CONTACT_STATUS:
			break;
	}
}
