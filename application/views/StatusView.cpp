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
#include <PopUpMenu.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/BitmapUtils.h>
#include <libinterface/BitmapView.h>
#include <libinterface/MenuButton.h>

#include "AccountManager.h"
#include "ChatProtocolMessages.h"
#include "ImageCache.h"
#include "NicknameTextControl.h"
#include "Server.h"
#include "StatusMenuItem.h"
#include "Utils.h"


const int32 kSetNickname = 'SVnk';
const int32 kSelectAllAccounts = 'SVaa';
const int32 kSelectAccount = 'SVsa';


StatusView::StatusView(const char* name, Server* server)
	:
	BView(name, B_WILL_DRAW),
	fServer(server)
{
	// Nick name
	fNickname = new NicknameTextControl("Nickname",
		new BMessage(kSetNickname));

	// Status menu
	fStatusMenu = new BPopUpMenu("-");

	// Add status menu items
	int32 s = STATUS_ONLINE;
	while (s >= STATUS_ONLINE && s < STATUS_STATUSES) {
		StatusMenuItem* item = new StatusMenuItem(UserStatusToString(
			(UserStatus)s), (UserStatus)s);
		fStatusMenu->AddItem(item);

		/*// Add items for custom messages
		if (s == STATUS_ONLINE || s == STATUS_DO_NOT_DISTURB) {
			item = new StatusMenuItem("Custom...", (UserStatus)s, true);
			fStatusMenu->AddItem(item);
			fStatusMenu->AddItem(new BSeparatorItem());
		}*/

		// Mark offline status by default
		if (s == STATUS_OFFLINE)
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
	fAvatar->SetBitmap(ImageCache::Get()->GetImage("kPersonIcon"));

	// Changing the account used
	fAccountsMenu = new BPopUpMenu("statusAccountsMenu", true, false);
	fAccountsButton = new MenuButton("statusAccountsButton", "", new BMessage());
	fAccountsButton->SetMenu(fAccountsMenu);
	_PopulateAccountMenu();

	BMessage selected(kSelectAllAccounts);
	MessageReceived(&selected);

	// Set layout
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.SetInsets(0)
			.Add(statusField)
			.Add(fAccountsButton)
		.End()
		.AddGroup(B_HORIZONTAL)
			.Add(fNickname)
			.Add(fAvatar)
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
			accountManager->SetStatus((UserStatus)status, "");
			break;
		}
		case kSelectAllAccounts:
		case kSelectAccount:
		{
			int32 index = msg->FindInt32("index");
			BitmapMenuItem* item = (BitmapMenuItem*)fAccountsMenu->ItemAt(index);
			BBitmap* bitmap = item->Bitmap();
			fAccountsButton->SetIcon(bitmap);
			break;
		}
		case IM_MESSAGE:
		{
			int32 im_what = msg->GetInt32("im_what", 0);
			if (im_what == IM_PROTOCOL_DISABLE || im_what == IM_PROTOCOL_READY)
				_PopulateAccountMenu();
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
StatusView::SetStatus(UserStatus status)
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
	// We don't want the default avatar to override a real one
	if (bitmap != ImageCache::Get()->GetImage("kPersonIcon"))
		fAvatar->SetBitmap(bitmap);
}


void
StatusView::_PopulateAccountMenu()
{
	AccountInstances accounts = fServer->GetActiveAccounts();
	BFont font;
	GetFont(&font);

	if (fAccountsMenu->FindItem(B_TRANSLATE("All")) == NULL) {
		BBitmap* icon = ImageCache::Get()->GetImage("kAsteriskIcon");
		BBitmap* resized = RescaleBitmap(icon, font.Size(), font.Size());

		fAccountsMenu->AddItem(new BitmapMenuItem(B_TRANSLATE("All"),
			new BMessage(kSelectAllAccounts), resized));
		fAccountsMenu->FindItem(B_TRANSLATE("All"))->SetMarked(true);
	}

	// Add unpopulated entries
	for (int i = 0; i < accounts.CountItems(); i++) {
		BString name = accounts.KeyAt(i);
		int64 instance = accounts.ValueAt(i);
		ProtocolLooper* looper = fServer->GetProtocolLooper(instance);
		if (looper == NULL || looper->Protocol() == NULL
				|| fAccountsMenu->FindItem(name.String()) != NULL)
			continue;

		BBitmap* icon = looper->Protocol()->Icon();
		BBitmap* resized = RescaleBitmap(icon, font.Size(), font.Size());

		BMessage* selected = new BMessage(kSelectAccount);
		selected->AddInt64("instance", instance);
		fAccountsMenu->AddItem(new BitmapMenuItem(name.String(), selected,
			resized));
	}

	// Remove disabled accounts
	if (fAccountsMenu->CountItems() - 1 > accounts.CountItems()) {
		fAccountsMenu->FindMarked()->SetMarked(false);
		fAccountsMenu->ItemAt(0)->SetMarked(true);
		BMessage select(kSelectAllAccounts);
		MessageReceived(&select);

		for (int i = 0; i < fAccountsMenu->CountItems(); i++) {
			bool found = false;
			accounts.ValueFor(BString(fAccountsMenu->ItemAt(i)->Label()), &found);
			if (found == false)
				fAccountsMenu->RemoveItem(i);
		}
	}
	fAccountsMenu->SetTargetForItems(this);
}
