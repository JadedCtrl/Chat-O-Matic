/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

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
#include "PreferencesAccounts.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PreferencesAccounts"


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
		ChatProtocolAddOn* addOn = pm->ProtocolAddOnAt(i);
		ProtocolSettings* settings = new ProtocolSettings(addOn);

		// Add accounts to list view
		_LoadListView(settings);

		// Add menu items
		BMessage* msg = new BMessage(kAddAccount);
		msg->AddPointer("settings", settings);

		BitmapMenuItem* item = new BitmapMenuItem(
			addOn->ProtoFriendlySignature(), msg, addOn->ProtoIcon());
		fProtosMenu->AddItem(item);
	}

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
			.SetInsets(0, 0, 0, 15)
			.Add(proto)
			.Add(fDelButton)
			.AddGlue()
			.Add(fToggleButton)
			.Add(fEditButton)
		.End()
	.End();
}


void
PreferencesAccounts::AttachedToWindow()
{
	fListView->SetTarget(this);
	fProtosMenu->SetTargetForItems(this);
	fDelButton->SetTarget(this);
	fEditButton->SetTarget(this);
	fToggleButton->SetTarget(this);
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
				fToggleButton->SetEnabled(index >= 0);

				if (index >= 0) {
					AccountListItem* item = (AccountListItem*)fListView->ItemAt(fListView->CurrentSelection());

					if (_AccountEnabled(item->Account() ) == true) {
						fToggleButton->SetLabel(B_TRANSLATE("Disable"));
						fToggleButton->SetEnabled(true);
					}
					else {
						fToggleButton->SetLabel(B_TRANSLATE("Enable"));
						fToggleButton->SetEnabled(false);
					}
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

			bool found = false;
			AccountInstances accs = ((TheApp*)be_app)->GetMainWindow()->GetServer()->GetAccounts();
			int64 instance = accs.ValueFor(BString(item->Account()), &found);
			if (found == false)
				return;

			BMessage* remove = new BMessage(IM_MESSAGE);
			remove->AddInt32("im_what", IM_PROTOCOL_DISABLE);
			remove->AddInt64("instance", instance);
			((TheApp*)be_app)->GetMainWindow()->PostMessage(remove);

			fToggleButton->SetLabel(B_TRANSLATE("Enable"));
			fToggleButton->SetEnabled(false);
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


bool
PreferencesAccounts::_AccountEnabled(const char* account)
{
	bool found = false;
	AccountInstances accs = ((TheApp*)be_app)->GetMainWindow()->GetServer()->GetAccounts();
	accs.ValueFor(BString(account), &found);

	return found;
}


