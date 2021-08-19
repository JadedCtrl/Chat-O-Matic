/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "AccountsWindow.h"

#include <Button.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <PopUpMenu.h>
#include <ScrollView.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/MenuButton.h>

#include "AccountDialog.h"
#include "AccountListItem.h"
#include "ChatProtocol.h"
#include "ChatProtocolMessages.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "AccountsWindow"


const uint32 kAddAccount	= 'adac';
const uint32 kEditAccount	= 'edac';
const uint32 kDelAccount	= 'dlac';
const uint32 kToggleAccount	= 'tgac';
const uint32 kSelect		= 'selt';


static int
compare_by_name(const void* _item1, const void* _item2)
{
	AccountListItem* item1 = *(AccountListItem**)_item1;
	AccountListItem* item2 = *(AccountListItem**)_item2;

	return strcasecmp(item1->Account(), item2->Account());
}


AccountsWindow::AccountsWindow()
	:
	BWindow(BRect(200, 200, 300, 400),
		B_TRANSLATE("Accounts"), B_FLOATING_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS)
{
	fListView = new BListView("accountsListView");
	fListView->SetInvocationMessage(new BMessage(kEditAccount));
	fListView->SetSelectionMessage(new BMessage(kSelect));
	fListView->SetExplicitMinSize(BSize(B_SIZE_UNSET, BFont().Size() * 20));

	BScrollView* scrollView = new BScrollView("scrollView", fListView,
		B_WILL_DRAW, false, true);

	ProtocolManager* pm = ProtocolManager::Get();

	fProtosMenu = new BPopUpMenu(NULL, true);
	BObjectList<BitmapMenuItem> accountItems;
	BObjectList<BitmapMenuItem> purpleItems;

	for (uint32 i = 0; i < pm->CountProtocolAddOns(); i++) {
		ChatProtocolAddOn* addOn = pm->ProtocolAddOnAt(i);
		ProtocolSettings* settings = new ProtocolSettings(addOn);

		// Add accounts to list view
		_LoadListView(settings);

		// Add menu items
		BMessage* msg = new BMessage(kAddAccount);
		msg->AddPointer("settings", settings);

		BitmapMenuItem* item = new BitmapMenuItem(
			addOn->ProtoFriendlySignature(), msg, addOn->ProtoIcon());

		if (BString(addOn->Signature()) == "purple")
			purpleItems.AddItem(item);
		else
			accountItems.AddItem(item);
	}

	for (int i = 0; i < accountItems.CountItems(); i++)
		fProtosMenu->AddItem(accountItems.ItemAt(i));
	fProtosMenu->AddSeparatorItem();

	for (int i = 0; i < purpleItems.CountItems(); i++)
		fProtosMenu->AddItem(purpleItems.ItemAt(i));
	fProtosMenu->SetTargetForItems(this);

	MenuButton* proto = new MenuButton("addButton", B_TRANSLATE("Add"), NULL);
	proto->SetMenu(fProtosMenu);
	fDelButton = new BButton(B_TRANSLATE_COMMENT("Del", "Short for 'delete'"),
		new BMessage(kDelAccount));
	fEditButton = new BButton(B_TRANSLATE("Edit" B_UTF8_ELLIPSIS),
		new BMessage(kEditAccount));
	fToggleButton = new BButton(B_TRANSLATE("Enable"),
		new BMessage(kToggleAccount));
	fDelButton->SetEnabled(false);
	fEditButton->SetEnabled(false);
	fToggleButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(scrollView)
		.AddGroup(B_HORIZONTAL)
			.Add(proto)
			.Add(fDelButton)
			.AddGlue()
			.Add(fToggleButton)
			.Add(fEditButton)
		.End()
	.End();

	CenterOnScreen();
}


void
AccountsWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kSelect: {
			int32 index;

			if (msg->FindInt32("index", &index) == B_OK) {
				fDelButton->SetEnabled(index >= 0);
				fEditButton->SetEnabled(index >= 0);
				fToggleButton->SetEnabled(index >= 0);

				if (index >= 0) {
					AccountListItem* item = (AccountListItem*)fListView->ItemAt(fListView->CurrentSelection());

					if (_AccountInstance(item->Account()) > -1)
						fToggleButton->SetLabel(B_TRANSLATE("Disable"));
					else
						fToggleButton->SetLabel(B_TRANSLATE("Enable"));
				}
			}
			break;
		}
		case kAddAccount: {
			void *pointer = NULL;
			if (msg->FindPointer("settings", &pointer) == B_OK) {
				ProtocolSettings* settings
					= reinterpret_cast<ProtocolSettings*>(pointer);
				if (settings) {
					AccountDialog* dialog = new AccountDialog(
						B_TRANSLATE("Add account"), settings);
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

				AccountDialog* dialog = new AccountDialog(
					B_TRANSLATE("Edit account"), item->Settings(),
					item->Account());
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
		case kToggleAccount: {
			int32 current = fListView->CurrentSelection();
			AccountListItem* item;

			if (current < 0
			|| (item = (AccountListItem*)fListView->ItemAt(current)) == NULL)
				break;

			const char* account = item->Account();
			ProtocolManager::Get()->ToggleAccount(item->Settings(), account);
			fListView->DeselectAll();
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

				ProtocolManager::Get()->EnableAccount(settings, account);
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
			BWindow::MessageReceived(msg);
	}
}


void
AccountsWindow::_LoadListView(ProtocolSettings* settings)
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


int64
AccountsWindow::_AccountInstance(const char* account)
{
	bool found = false;
	AccountInstances accs =
		((TheApp*)be_app)->GetMainWindow()->GetServer()->GetAccounts();
	int64 instance = accs.ValueFor(BString(account), &found);

	if (found == false)
		instance = -1;
	return instance;
}
