/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "AccountsMenu.h"

#include <Catalog.h>
#include <MenuItem.h>

#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AccountsMenu"


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
AccountsMenu::_PopulateMenu()
{
	if (CountItems() > 0)
		RemoveItems(0, CountItems(), true);
	
	if (fAllMessage != NULL)
		AddItem(new BMenuItem(B_TRANSLATE("All"), new BMessage(*fAllMessage)));

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	AccountInstances accounts = server->GetActiveAccounts();

	for (int i = 0; i < accounts.CountItems(); i++) {
		BString label = accounts.KeyAt(i).String();
		if (label.CountChars() > 15) {
			label.RemoveChars(16, label.CountChars() - 16);
			label << B_UTF8_ELLIPSIS;
		}
		AddItem(new BMenuItem(label.String(), new BMessage(fAccountMessage)));
	}

	if (CountItems() > 0)
		ItemAt(0)->SetMarked(true);
	else
		SetEnabled(false);
}
