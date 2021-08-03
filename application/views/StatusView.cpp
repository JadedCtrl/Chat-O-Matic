/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "StatusView.h"

#include <Bitmap.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <PopUpMenu.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/BitmapUtils.h>
#include <libinterface/BitmapView.h>
#include <libinterface/EnterTextView.h>
#include <libinterface/MenuButton.h>

#include "AccountManager.h"
#include "AccountsMenu.h"
#include "ChatProtocolMessages.h"
#include "Contact.h"
#include "ImageCache.h"
#include "NotifyMessage.h"
#include "Server.h"
#include "StatusMenuItem.h"
#include "Utils.h"


const int32 kSelectAccount = 'SVsa';
const int32 kSetNick = 'SVsn';


StatusView::StatusView(const char* name, Server* server)
	:
	BView(name, B_WILL_DRAW),
	fServer(server),
	fAccount(-1)
{
	// Nick name
	fNickname = new EnterTextView("nicknameTextView");
	fNickname->MakeEditable(true);
	fNickname->MakeResizable(true);
	fNickname->SetTarget(this);
	fNickname->SetMessage(BMessage(kSetNick), "nick");

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
	fAccountsMenu = new AccountsMenu("statusAccountsMenu",
		BMessage(kSelectAccount), new BMessage(kSelectAccount), fServer);
	fAccountsButton = new MenuButton("statusAccountsButton", "", new BMessage());
	fAccountsButton->SetMenu(fAccountsMenu);

	BMessage selected(kSelectAccount);
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
		case kSetNick:
		{
			BString nick;
			if (msg->FindString("nick", &nick) == B_OK)
				AccountManager::Get()->SetNickname(nick, fAccount);
			_SetToAccount();
			break;
		}
		case kSetStatus:
		{
			int32 status;
			if (msg->FindInt32("status", &status) == B_OK)
				AccountManager::Get()->SetStatus((UserStatus)status, "",
					fAccount);
			_SetToAccount();
			break;
		}
		case kSelectAccount:
		{
			int32 index = msg->GetInt32("index", 0);
			BitmapMenuItem* item = (BitmapMenuItem*)fAccountsMenu->ItemAt(index);

			// Set button icon/label
			fAccountsButton->SetLabel("");
			BBitmap* bitmap = item->Bitmap();
			fAccountsButton->SetIcon(bitmap);
			if (bitmap == NULL) {
				char label[2] = { item->Label()[0], '\0' };
				fAccountsButton->SetLabel(label);
			}

			fAccount = msg->GetInt64("instance", -1);
			_SetToAccount();
			break;
		}
		case IM_MESSAGE: {
			int32 im_what = msg->GetInt32("im_what", 0);
			if (im_what == IM_PROTOCOL_READY || im_what == IM_PROTOCOL_DISABLE)
				fAccountsMenu->SetTargetForItems(this);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void
StatusView::ObserveString(int32 what, BString str)
{
	if (what == STR_CONTACT_NAME || what == STR_PERSONAL_STATUS)
		_SetToAccount();
}


void
StatusView::ObserveInteger(int32 what, int32 value)
{
	if (what == INT_ACCOUNT_STATUS || what == INT_CONTACT_STATUS)
		_SetToAccount();
}


void
StatusView::ObservePointer(int32 what, void* ptr)
{
	if (what == PTR_AVATAR_BITMAP)
		_SetToAccount();
}


void
StatusView::_SetToAccount()
{
	int64 instance = fAccount;
	if (instance == -1)
		instance = fServer->GetActiveAccounts().ValueAt(0);

	ProtocolLooper* looper = fServer->GetProtocolLooper(instance);
	if (looper == NULL || looper->GetOwnContact() == NULL)
		return;
	Contact* contact = looper->GetOwnContact();

	_SetAvatarIcon(contact->AvatarBitmap());
	_SetName(contact->GetName());
	_SetStatus(contact->GetNotifyStatus());
}


void	
StatusView::_SetName(BString name)
{
	fNickname->SetText(name.String());	
}


void
StatusView::_SetStatus(UserStatus status)
{
	for (int32 i = 0; i < fStatusMenu->CountItems(); i++) {
		StatusMenuItem* item
			= reinterpret_cast<StatusMenuItem*>(fStatusMenu->ItemAt(i));
		if (item && item->Status() == status && !item->IsCustom())
			item->SetMarked(true);
	}
}


void
StatusView::_SetAvatarIcon(const BBitmap* bitmap)
{
	fAvatar->SetBitmap(bitmap);
}
