/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Casalinuovo Dario
 */

#include "UserInfoWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Box.h>
#include <GridLayout.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <Layout.h>
#include <Message.h>
#include <SpaceLayoutItem.h>
#include <String.h>

#include <libinterface/BitmapView.h>

#include "AppMessages.h"
#include "ChatProtocolMessages.h"
#include "AppConstants.h"
#include "RenderView.h"
#include "Utils.h"
#include "NotifyMessage.h"
#include "User.h"


UserInfoWindow::UserInfoWindow(User* user)
	:
	BWindow(BRect(200, 200, 500, 400),
		"User information", B_FLOATING_WINDOW,
		B_NOT_ZOOMABLE | B_NOT_RESIZABLE),
		fUser(user)
{
	fPersonalMessage = new BTextView("personalMessage", B_WILL_DRAW);
	fPersonalMessage->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT,
						 B_ALIGN_MIDDLE));

	fPersonalMessage->SetText(fUser->GetNotifyPersonalStatus());
	fPersonalMessage->SetExplicitMaxSize(BSize(200, 200));
	fPersonalMessage->MakeEditable(false);
	fPersonalMessage->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	BString status(fUser->GetName());
	status << UserStatusToString(fUser->GetNotifyStatus());

	status << "\n\n ID : ";
	status << fUser->GetId();

	fStatus = new BTextView("status", B_WILL_DRAW);
	fStatus->SetText(status);
	fStatus->MakeEditable(false);
	fStatus->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	SetLayout(new BGroupLayout(B_HORIZONTAL));

	fAvatar = new BitmapView("UserIcon");
	fAvatar->SetExplicitMaxSize(BSize(70, 70));
	fAvatar->SetExplicitMinSize(BSize(50, 50));
	fAvatar->SetExplicitPreferredSize(BSize(50, 50));
	fAvatar->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	fAvatar->SetBitmap(fUser->AvatarBitmap());

	AddChild(BGroupLayoutBuilder(B_VERTICAL, 10)
		.AddGroup(B_HORIZONTAL)
			.Add(fStatus)
			.Add(fAvatar)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(fPersonalMessage)
		.End()
		.SetInsets(5, 5, 5, 5)
	);

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
