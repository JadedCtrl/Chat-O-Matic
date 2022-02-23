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
#include "Server.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AccountsMenu"


int64 AccountsMenu::fDefaultSelection = -1;


AccountsMenu::AccountsMenu(const char* name, BMessage msg, BMessage* allMsg)
	:
	BPopUpMenu(name),
	fAccountMessage(msg),
	fAllMessage(allMsg)
{
	_PopulateMenu();
	SetRadioMode(true);
	SetLabelFromMarked(true);
	Server::Get()->RegisterObserver(this);
}


AccountsMenu::~AccountsMenu()
{
	delete fAllMessage;
	Server::Get()->UnregisterObserver(this);
}


void
AccountsMenu::ObserveInteger(int32 what, int32 value)
{
	_PopulateMenu();
}


void
AccountsMenu::SetDefaultSelection(BMenuItem* item)
{
	fDefaultSelection = item->Message()->GetInt64("instance", -1);
}


void
AccountsMenu::_PopulateMenu()
{
	// Add 'all' item if missing
	if (fAllMessage != NULL && FindItem(B_TRANSLATE("All")) == NULL) {
		BBitmap* icon = _EnsureAsteriskIcon();
		AddItem(new BitmapMenuItem(B_TRANSLATE("All"), new BMessage(*fAllMessage),
			icon, 0, 0, false));
	}

	AccountInstances accounts = Server::Get()->GetActiveAccounts();

	// Add protocol item if not already in menu
	for (int i = 0; i < accounts.CountItems(); i++) {
		int64 instance = accounts.ValueAt(i);
		// Initialize default selection if necessary
		if (fDefaultSelection == -1)
			fDefaultSelection = instance;

		BString label = accounts.KeyAt(i).String();
		if (label.CountChars() > 15) {
			label.RemoveChars(16, label.CountChars() - 16);
			label << B_UTF8_ELLIPSIS;
		}

		if (FindItem(label.String()) != NULL)
			continue;

		ProtocolLooper* looper = Server::Get()->GetProtocolLooper(instance);
		BBitmap* icon = _EnsureProtocolIcon(label.String(), looper);

		BMessage* message = new BMessage(fAccountMessage);
		message->AddInt64("instance", instance);
		AddItem(new AccountMenuItem(label.String(), message, icon));
	}

	// If an account has been disabled since last population… get ridda it
	if ((fAllMessage != NULL && CountItems() - 1 > accounts.CountItems())
			|| (fAllMessage == NULL && CountItems() > accounts.CountItems()))
		for (int i = 0; i < CountItems(); i++) {
			bool found = false;
			int64 instance = ItemAt(i)->Message()->GetInt64("instance", 0);
			for (int j = 0; j < accounts.CountItems(); j++)
				if (accounts.ValueAt(j) == instance)
					found = true;

			if (fAllMessage != NULL && i == 0)
				continue;
			if (found == false)
				RemoveItem(i);
		}

	// Deselect all
	for (int i = 0; i < CountItems(); i++)
		ItemAt(i)->SetMarked(false);

	// Apply last/default selection
	BMenuItem* selection = ItemAt(0);
	if (fAllMessage == NULL)
		for (int i = 0; i < CountItems(); i++) {
			BMenuItem* item = ItemAt(i);
			if (item->Message()->GetInt64("instance", -1) == fDefaultSelection)
				selection = item;
		}
	selection->SetMarked(true);
}


BBitmap*
AccountsMenu::_EnsureProtocolIcon(const char* label, ProtocolLooper* looper)
{
	BFont font;
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
	BBitmap* icon = ImageCache::Get()->GetImage("kAsteriskScaled");

	if (icon == NULL) {
		BBitmap* unscaled = ImageCache::Get()->GetImage("kAsteriskIcon");
		icon = RescaleBitmap(unscaled, font.Size(), font.Size());
		ImageCache::Get()->AddImage("kAsteriskScaled", icon);
	}
	return icon;
}
