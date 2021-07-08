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
#include <LayoutBuilder.h>
#include <Message.h>

#include <libinterface/BitmapView.h>

#include "Utils.h"
#include "User.h"


UserInfoWindow::UserInfoWindow(User* user)
	:
	BWindow(BRect(200, 200, 500, 400),
		"User information", B_FLOATING_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
		fUser(user)
{
	_InitInterface();
	MoveTo(BAlert::AlertPosition(Bounds().Width(), Bounds().Height() / 2));
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
UserInfoWindow::_InitInterface()
{
	fNameLabel = new BStringView("nameLabel", fUser->GetName().String());
	fStatusLabel = new BStringView("statusLabel",
		UserStatusToString(fUser->GetNotifyStatus()));
	fTextStatusLabel = new BStringView("statusMessageLabel",
		fUser->GetNotifyPersonalStatus());

	BString idText("[");
	idText << fUser->GetId().String() << "]";
	fIdLabel = new BStringView("idLabel", idText.String());

	fAvatar = new BitmapView("UserIcon");
	fAvatar->SetExplicitMaxSize(BSize(70, 70));
	fAvatar->SetExplicitMinSize(BSize(50, 50));
	fAvatar->SetExplicitPreferredSize(BSize(50, 50));
	fAvatar->SetBitmap(fUser->AvatarBitmap());

	// Centering is lyfeee
	fNameLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fIdLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fStatusLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fAvatar->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));

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
			.Add(fStatusLabel)
			.AddGlue()
		.End()
	.End();
}
