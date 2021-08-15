/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Casalinuovo Dario
 *		Jaidyn Levesque <jadedctrl@teknik.io>
 */

#include "UserInfoWindow.h"

#include <Alert.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Message.h>

#include <libinterface/BitmapView.h>

#include "ImageCache.h"
#include "NotifyMessage.h"
#include "User.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "UserInfoWindow"


UserInfoWindow::UserInfoWindow(User* user)
	:
	BWindow(BRect(200, 200, 300, 400),
		B_TRANSLATE("User information"), B_FLOATING_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	fUser(user)
{
	_InitInterface();
	MoveTo(BAlert::AlertPosition(Bounds().Width(), Bounds().Height() / 2));

	fUser->RegisterObserver(this);
}


UserInfoWindow::~UserInfoWindow()
{
	fUser->UnregisterObserver(this);
}


void
UserInfoWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
UserInfoWindow::ObserveString(int32 what, BString string)
{
	Lock();
	switch (what) {
		case STR_CONTACT_NAME:
			fNameLabel->SetText(string);
			break;
		case STR_PERSONAL_STATUS:
			fTextStatusLabel->SetText(string);
			break;
	}
	Unlock();
}


void
UserInfoWindow::ObserveInteger(int32 what, int32 num)
{
	Lock();
	switch (what) {
		case INT_CONTACT_STATUS:
			_UpdateStatusViews((UserStatus)num);
			break;
	}
	Unlock();
}


void
UserInfoWindow::ObservePointer(int32 what, void* ptr)
{
	Lock();
	switch (what) {
		case PTR_AVATAR_BITMAP:
			fAvatar->SetBitmap((BBitmap*)ptr);
			break;
	}
	Unlock();
}



void
UserInfoWindow::_InitInterface()
{
	fNameLabel = new BStringView("nameLabel", fUser->GetName().String());
	fNameLabel->SetFont(be_bold_font);

	fStatusLabel = new BStringView("statusLabel", "");

	float iconSize = be_plain_font->Size() + 5;
	fStatusIcon = new BitmapView("statusIcon");
	fStatusIcon->SetExplicitMaxSize(BSize(iconSize, iconSize));

	fTextStatusLabel = new BStringView("statusMessageLabel",
		fUser->GetNotifyPersonalStatus());

	_UpdateStatusViews(fUser->GetNotifyStatus());

	const char* userId = fUser->GetId().String();
	fIdLabel = new BTextView("idLabel");
	fIdLabel->SetText(userId);
	fIdLabel->SetFont(be_fixed_font);
	fIdLabel->SetWordWrap(false);
	fIdLabel->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fIdLabel->MakeEditable(false);
	fIdLabel->SetExplicitMinSize(
		BSize(be_fixed_font->StringWidth(userId), B_SIZE_UNSET));

	fAvatar = new BitmapView("userIcon");
	fAvatar->SetExplicitMaxSize(BSize(70, 70));
	fAvatar->SetExplicitMinSize(BSize(50, 50));
	fAvatar->SetExplicitPreferredSize(BSize(50, 50));
	fAvatar->SetBitmap(fUser->AvatarBitmap());

	// Centering is lyfeee
	fNameLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fIdLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fStatusIcon->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_TOP));
	fStatusLabel->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_TOP));
	fAvatar->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_TOP));

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 10)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.AddGroup(B_VERTICAL)
			.Add(fNameLabel)
			.Add(fIdLabel)
			.Add(fTextStatusLabel)
			.AddGlue()
		.End()
		.AddGroup(B_VERTICAL)
		.AddGroup(B_VERTICAL)
			.Add(fAvatar)
			.AddGroup(B_HORIZONTAL)
				.Add(fStatusIcon)
				.Add(fStatusLabel)
			.End()
		.End()
	.End();
}


void
UserInfoWindow::_UpdateStatusViews(UserStatus status)
{
	fStatusLabel->SetText(UserStatusToString(status));

	BBitmap* statusBitmap =
		ImageCache::Get()->GetImage(UserStatusToImageKey(status));
	fStatusIcon->SetBitmap(statusBitmap);
}
