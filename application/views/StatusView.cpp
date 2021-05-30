/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "StatusView.h"

#include <Bitmap.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <StringView.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/BitmapUtils.h>
#include <libinterface/BitmapView.h>

#include "AccountManager.h"
#include "CayaUtils.h"
#include "NicknameTextControl.h"
#include "StatusMenuItem.h"


const int32 kSetNickname = 'stnk';


StatusView::StatusView(const char* name)
	:
	BView(name, B_WILL_DRAW)
{
	// Nick name
	fNickname = new NicknameTextControl("Nickname",
		new BMessage(kSetNickname));

	// Status menu
	fStatusMenu = new BPopUpMenu("-");

	// Add status menu items
	int32 s = CAYA_ONLINE;
	while (s >= CAYA_ONLINE && s < CAYA_STATUSES) {
		StatusMenuItem* item = new StatusMenuItem(CayaStatusToString(
			(CayaStatus)s), (CayaStatus)s);
		fStatusMenu->AddItem(item);

		/*// Add items for custom messages
		if (s == CAYA_ONLINE || s == CAYA_DO_NOT_DISTURB) {
			item = new StatusMenuItem("Custom...", (CayaStatus)s, true);
			fStatusMenu->AddItem(item);
			fStatusMenu->AddItem(new BSeparatorItem());
		}*/

		// Mark offline status by default
		if (s == CAYA_OFFLINE)
			item->SetMarked(true);

		s++;
	}

	// Menu field
	BMenuField* statusField = new BMenuField("StatusField", NULL,
		fStatusMenu);

	// Icon
	fAvatar = new BitmapView("AvatarIcon");
	fAvatar->SetExplicitMaxSize(BSize(50, 50));
	fAvatar->SetExplicitPreferredSize(BSize(50, 50));

	// Set layout
	BLayoutBuilder::Group<>(this, B_HORIZONTAL)
		.AddGroup(B_VERTICAL)
			.Add(statusField)
			.AddGroup(B_HORIZONTAL)
				.Add(fNickname)
				.Add(fAvatar)
			.End()
		.End()
	.End();
}


void
StatusView::AttachedToWindow()
{
	//fNickname->SetTarget(this);
	fStatusMenu->SetTargetForItems(this);
}


void
StatusView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kSetNickname:
		{
			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetNickname(fNickname->Text());
			break;
		}
		case kSetStatus:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return;

			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetStatus((CayaStatus)status, "");
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void	
StatusView::SetName(BString name)
{
	fNickname->SetText(name.String());	
}


void
StatusView::SetStatus(CayaStatus status)
{
	for (int32 i = 0; i < fStatusMenu->CountItems(); i++) {
		StatusMenuItem* item
			= reinterpret_cast<StatusMenuItem*>(fStatusMenu->ItemAt(i));
		if (item && item->Status() == status && !item->IsCustom())
			item->SetMarked(true);
	}
}


void
StatusView::SetAvatarIcon(const BBitmap* bitmap)
{
	fAvatar->SetBitmap(bitmap);
}
