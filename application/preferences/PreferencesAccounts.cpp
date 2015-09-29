/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Button.h>
#include <ControlLook.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ListView.h>
#include <PopUpMenu.h>
#include <ScrollView.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/ToolButton.h>

#include "AccountDialog.h"
#include "AccountListItem.h"
#include "CayaProtocol.h"
#include "PreferencesAccounts.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "TheApp.h"

const uint32 kAddAccount   = 'adac';
const uint32 kEditAccount  = 'edac';
const uint32 kDelAccount   = 'dlac';
const uint32 kSelect       = 'selt';


static int
compare_by_name(const void* _item1, const void* _item2)
{
	AccountListItem* item1 = *(AccountListItem**)_item1;
	AccountListItem* item2 = *(AccountListItem**)_item2;

	return strcasecmp(item1->Account(), item2->Account());
}


PreferencesAccounts::PreferencesAccounts()
	: BView("Accounts", B_WILL_DRAW)
{
	fListView = new BListView("accountsListView");
	fListView->SetInvocationMessage(new BMessage(kEditAccount));
	fListView->SetSelectionMessage(new BMessage(kSelect));

	BScrollView* scrollView = new BScrollView("scrollView", fListView,
		B_WILL_DRAW, false, true);

	ProtocolManager* pm = ProtocolManager::Get();

	fProtosMenu = new BPopUpMenu(NULL, true);
	for (uint32 i = 0; i < pm->CountProtocolAddOns(); i++) {
		CayaProtocolAddOn* addOn = pm->ProtocolAddOnAt(i);
		ProtocolSettings* settings = new ProtocolSettings(addOn);

		// Add accounts to list view
		_LoadListView(settings);

		// Add menu items
		BMessage* msg = new BMessage(kAddAccount);
		msg->AddPointer("settings", settings);

		BitmapMenuItem* item = new BitmapMenuItem(
			addOn->FriendlySignature(), msg, addOn->Icon());
		fProtosMenu->AddItem(item);
	}

	ToolButton* proto = new ToolButton("Add", NULL);
	proto->SetMenu(fProtosMenu);
	fDelButton = new ToolButton("Del", new BMessage(kDelAccount));
	fEditButton = new ToolButton("Edit...", new BMessage(kEditAccount));
	fDelButton->SetEnabled(false);
	fEditButton->SetEnabled(false);

	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_HORIZONTAL, spacing));
	AddChild(BGroupLayoutBuilder(B_VERTICAL)
		.Add(scrollView)
		.AddGroup(B_HORIZONTAL, spacing)
			.Add(proto)
			.Add(fDelButton)
			.AddGlue()
			.Add(fEditButton)
		.End()
		.SetInsets(spacing, spacing, spacing, spacing)
	);
}


void
PreferencesAccounts::AttachedToWindow()
{
	fListView->SetTarget(this);
	fProtosMenu->SetTargetForItems(this);
	fDelButton->SetTarget(this);
	fEditButton->SetTarget(this);
}


void
PreferencesAccounts::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kSelect: {
				int32 index;

				if (msg->FindInt32("index", &index) == B_OK) {
					fDelButton->SetEnabled(index >= 0);
					fEditButton->SetEnabled(index >= 0);
				}
			}
			break;
		case kAddAccount: {
			void *pointer = NULL;
			if (msg->FindPointer("settings", &pointer) == B_OK) {
				ProtocolSettings* settings
					= reinterpret_cast<ProtocolSettings*>(pointer);
				if (settings) {
					AccountDialog* dialog = new AccountDialog("Add account",
						settings);
					dialog->SetTarget(this);
					dialog->Show();
				}
			}
			break;
		}
		case kEditAccount: {
			for (int32 i = 0; i < fListView->CountItems(); i++) {
				int32 selected = fListView->CurrentSelection(i);
				if (selected < 0)
					continue;

				AccountListItem* item
					= dynamic_cast<AccountListItem*>(fListView->ItemAt(selected));

				AccountDialog* dialog = new AccountDialog("Edit account",
					item->Settings(), item->Account());
				dialog->SetTarget(this);
				dialog->Show();
			}
			break;
		}
		case kDelAccount: {
			for (int32 i = 0; i < fListView->CountItems(); i++) {
				int32 selected = fListView->CurrentSelection(i);
				if (selected < 0)
					continue;

				AccountListItem* item
					= dynamic_cast<AccountListItem*>(fListView->ItemAt(selected));

				const char* account = item->Account();
				ProtocolSettings* settings = item->Settings();
				if (settings->Delete(account) == B_OK)
					fListView->RemoveItem(item);
				delete settings;
			}
			break;
		}
		case kAccountAdded:
		case kAccountRenamed: {
			void* pointer = NULL;
			BString account;
			BString account2;

			if (msg->FindPointer("settings", &pointer) != B_OK)
				return;
			if (msg->what == kAccountAdded) {
				if (msg->FindString("account", &account) != B_OK)
					return;
			} else {
				if (msg->FindString("from", &account) != B_OK)
					return;
				if (msg->FindString("to", &account2) != B_OK)
					return;
			}

			ProtocolSettings* settings
				= reinterpret_cast<ProtocolSettings*>(pointer);
			if (!settings)
				return;

			if (msg->what == kAccountAdded) {
				// Add list item
				AccountListItem* listItem
					= new AccountListItem(settings, account.String());
				fListView->AddItem(listItem);

				// Add protocol/account instance
				TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
				ProtocolManager::Get()->AddAccount(settings->AddOn(),
					account.String(), theApp->GetMainWindow());
			} else {
				// Rename list item
				for (int32 i = 0; i < fListView->CountItems(); i++) {
					AccountListItem* listItem
						= dynamic_cast<AccountListItem*>(fListView->ItemAt(i));
					if (!listItem)
						continue;

					if (account == listItem->Account()) {
						listItem->SetAccount(account2.String());
						break;
					}
				}
			}

			fListView->SortItems(compare_by_name);
			break;
		}
		default:
			BView::MessageReceived(msg);
	}
}


void
PreferencesAccounts::_LoadListView(ProtocolSettings* settings)
{
	if (!settings)
		return;

	BObjectList<BString> accounts = settings->Accounts();

	// Add accounts to list view
	for (int32 i = 0; i < accounts.CountItems(); i++) {
		BString* account = accounts.ItemAt(i);
		AccountListItem* listItem
			= new AccountListItem(settings, account->String());
		fListView->AddItem(listItem);
	}
}
