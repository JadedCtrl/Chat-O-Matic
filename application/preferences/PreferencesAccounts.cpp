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

const uint32 kAddAccount   = 'ADAC';
const uint32 kEditAccount  = 'EDAC';
const uint32 kDelAccount   = 'DLAC';
const uint32 kSelect       = 'SELT';


PreferencesAccounts::PreferencesAccounts()
	: BView("Accounts", B_WILL_DRAW)
{
	fListView = new BListView("accountsListView");
	fListView->SetInvocationMessage(new BMessage(kEditAccount));
	fListView->SetSelectionMessage(new BMessage(kSelect));

	BScrollView* scrollView = new BScrollView("scrollView", fListView,
		B_WILL_DRAW, false, true);

	BList* protocols = ProtocolManager::Get()->GetProtocols();

	fProtosMenu = new BPopUpMenu(NULL, true);
	for (int32 i = 0; i < protocols->CountItems(); i++) {
		CayaProtocol* cayap
			= reinterpret_cast<CayaProtocol*>(protocols->ItemAtFast(i));
		ProtocolSettings* settings = new ProtocolSettings(cayap);

		// Add accounts to list view
		_LoadListView(settings);

		// Add menu items
		BMessage* msg = new BMessage(kAddAccount);
		msg->AddPointer("protocol", cayap);

		BitmapMenuItem* item = new BitmapMenuItem(
			cayap->GetFriendlySignature(), msg,
			ProtocolManager::Get()->GetProtocolIcon(cayap->GetSignature()));
		fProtosMenu->AddItem(item);

		delete settings;
	}

	ToolButton* proto = new ToolButton("+", NULL);
	proto->SetMenu(fProtosMenu);
	fDelButton = new ToolButton("-", new BMessage(kDelAccount));
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
			void *protocol = NULL;
			if (msg->FindPointer("protocol", &protocol) == B_OK) {
				CayaProtocol* cayap = (CayaProtocol*) protocol;

				BLooper* looper = new BLooper();
				looper->AddHandler(this);

				AccountDialog* dialog = new AccountDialog("Add account", cayap);
				dialog->Show();
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

				CayaProtocol* cayap = item->Protocol();
				const char* account = item->Account();

				BLooper* looper = new BLooper();
				looper->AddHandler(this);

				AccountDialog* dialog = new AccountDialog("Edit account", cayap, account);
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
		default:
			BView::MessageReceived(msg);
	}
}


void
PreferencesAccounts::_LoadListView(ProtocolSettings* settings)
{
	if (!settings)
		return;

	List<BString> accounts = settings->Accounts();

	// Add accounts to list view
	for (uint32 i = 0; i < accounts.CountItems(); i++) {
		BString account = accounts.ItemAt(i);
		AccountListItem* listItem = new AccountListItem(
			settings->Protocol(), account.String());
		fListView->AddItem(listItem);
	}
}
