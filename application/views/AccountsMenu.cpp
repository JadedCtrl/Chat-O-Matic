/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "AccountsMenu.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"

#include <MenuItem.h>


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
		AddItem(new BMenuItem("All", new BMessage(*fAllMessage)));

	Server* server = ((TheApp*)be_app)->GetMainWindow()->GetServer();
	AccountInstances accounts = server->GetActiveAccounts();

	for (int i = 0; i < accounts.CountItems(); i++)
		AddItem(new BMenuItem(accounts.KeyAt(i).String(),
			new BMessage(fAccountMessage)));

	if (CountItems() > 0)
		ItemAt(0)->SetMarked(true);
	else
		SetEnabled(false);
}
