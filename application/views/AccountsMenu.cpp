/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "AccountsMenu.h"

#include <Bitmap.h>
#include <Catalog.h>
#include <MenuItem.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/BitmapUtils.h>

#include "AccountMenuItem.h"
#include "ImageCache.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AccountsMenu"


int32 AccountsMenu::fDefaultSelection = 0;


AccountsMenu::AccountsMenu(const char* name, BMessage msg, BMessage* allMsg)
	:
	BMenu(name),
	fAccountMessage(msg),
	fAllMessage(allMsg)
{
	_PopulateMenu();

	SetRadioMode(true);
	SetLabelFromMarked(true);

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	server->RegisterObserver(this);
}


AccountsMenu::~AccountsMenu()
{
	delete fAllMessage;
	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	server->UnregisterObserver(this);
}


void
AccountsMenu::ObserveInteger(int32 what, int32 value)
{
	_PopulateMenu();
}


void
AccountsMenu::SetDefaultSelection(BMenuItem* item)
{
	fDefaultSelection = IndexOf(item);
	if (fAllMessage != NULL)
		fDefaultSelection--;
}


void
AccountsMenu::_PopulateMenu()
{
	BFont font;
	GetFont(&font);

	// Add 'all' item if missing
	if (fAllMessage != NULL && FindItem(B_TRANSLATE("All")) == NULL) {
		BBitmap* icon = _EnsureAsteriskIcon();
		AddItem(new BitmapMenuItem(B_TRANSLATE("All"), new BMessage(*fAllMessage),
			icon, 0, 0, false));
	}

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	AccountInstances accounts = server->GetActiveAccounts();

	// Add protocol item if not already in menu
	for (int i = 0; i < accounts.CountItems(); i++) {
		int64 instance = accounts.ValueAt(i);
		BString label = accounts.KeyAt(i).String();
		if (label.CountChars() > 15) {
			label.RemoveChars(16, label.CountChars() - 16);
			label << B_UTF8_ELLIPSIS;
		}

		if (FindItem(label.String()) != NULL)
			continue;

		ProtocolLooper* looper = server->GetProtocolLooper(instance);
		BBitmap* icon = _EnsureProtocolIcon(label.String(), looper);

		BMessage* message = new BMessage(fAccountMessage);
		message->AddInt64("instance", instance);
		AddItem(new AccountMenuItem(label.String(), message, icon));
	}

	int32 selection = fDefaultSelection;

	// If an account has been disabled since last population… get ridda it
	if ((fAllMessage != NULL && CountItems() - 1 > accounts.CountItems())
			|| (fAllMessage == NULL && CountItems() > accounts.CountItems()))
		for (int i = 0; i < CountItems(); i++) {
			bool found = false;
			int64 instance = ItemAt(i)->Message()->GetInt64("instance", 0);
			for (int j = 0; j < accounts.CountItems(); j++)
				if (accounts.ValueAt(j) == instance)
					found = true;

			if (found == false)
				RemoveItem(i);
		}

	// Apply last/default selection
	if (fAllMessage == NULL && selection < CountItems() && selection >= 0)
		ItemAt(selection)->SetMarked(true);
	else if (CountItems() > 0)
		ItemAt(0)->SetMarked(true);
	else
		SetEnabled(false);
}


BBitmap*
AccountsMenu::_EnsureProtocolIcon(const char* label, ProtocolLooper* looper)
{
	BFont font;
	GetFont(&font);
	BBitmap* icon = ImageCache::Get()->GetImage(label);

	if (icon == NULL && looper != NULL && looper->Protocol()->Icon() != NULL) {
		BBitmap* unscaled = looper->Protocol()->Icon();
		icon = RescaleBitmap(unscaled, font.Size(), font.Size());
		ImageCache::Get()->AddImage(label, icon);
	}
	return icon;
}


BBitmap*
AccountsMenu::_EnsureAsteriskIcon()
{
	BFont font;
	GetFont(&font);
	BBitmap* icon = ImageCache::Get()->GetImage("kAsteriskScaled");

	if (icon == NULL) {
		BBitmap* unscaled = ImageCache::Get()->GetImage("kAsteriskIcon");
		icon = RescaleBitmap(unscaled, font.Size(), font.Size());
		ImageCache::Get()->AddImage("kAsteriskScaled", icon);
	}
	return icon;
}
