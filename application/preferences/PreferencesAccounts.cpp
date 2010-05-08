/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Button.h>
#include <ControlLook.h>
#include <CheckBox.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <ListView.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <Window.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/Divider.h>
#include <libinterface/NotifyingTextView.h>
#include <libinterface/ToolButton.h>

#include "AccountListItem.h"
#include "CayaProtocol.h"
#include "PreferencesAccounts.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"

const uint32 kAddAccount   = 'ADAC';
const uint32 kEditAccount  = 'EDAC';
const uint32 kDelAccount   = 'DLAC';
const uint32 kSelect       = 'SELT';

const uint32 kCancel       = 'CANC';
const uint32 kOK           = 'SAVE';

const uint32 kChanged      = 'CHGD';


class AccountView : public BView {
public:
	AccountView(const char* name)
		: BView(name, B_WILL_DRAW)
	{
	}

	void AttachedToWindow()
	{
		// Once we are attached to window, the GUI is already created
		// so we can set our window as target for messages
		for (int32 i = 0; i < CountChildren(); i++) {
			BView* child = ChildAt(i);

			BMenu* menu = dynamic_cast<BMenu*>(child);
			BMenuField* menuField
				= dynamic_cast<BMenuField*>(child);
			BTextControl* textControl
				= dynamic_cast<BTextControl*>(child);
			NotifyingTextView* textView
				= dynamic_cast<NotifyingTextView*>(child);
			BCheckBox* checkBox = dynamic_cast<BCheckBox*>(child);

			if (menuField)
				menu = menuField->Menu();

			if (menu) {
				for (int32 j = 0; j < menu->CountItems(); j++) {
					BMenuItem* item = menu->ItemAt(j);
					item->SetMessage(new BMessage(kChanged));
					item->SetTarget(Window());
				}

				menu->SetTargetForItems(Window());
			}

			if (textControl) {
				textControl->SetMessage(new BMessage(kChanged));
				textControl->SetTarget(Window());
			}

			if (checkBox) {
				checkBox->SetMessage(new BMessage(kChanged));
				checkBox->SetTarget(Window());
			}

			if (textView) {
				textView->SetMessage(new BMessage(kChanged));
				textView->SetTarget(Window());
			}
		}
	}
};


class AccountDialog : public BWindow {
public:
	AccountDialog(const char* title, CayaProtocol* cayap, const char* account = NULL)
		: BWindow(BRect(0, 0, 1, 1), title, B_MODAL_WINDOW, B_NOT_RESIZABLE |
			B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
	{
		fSettings = new ProtocolSettings(cayap);

		fAccountName = new BTextControl("accountName", "Account name:", NULL, NULL);
		fAccountName->SetFont(be_bold_font);
		if (account) {
			fAccountName->SetText(account);
			fAccountName->SetEnabled(false);
		} else
			fAccountName->MakeFocus(true);

		Divider* divider = new Divider("divider", B_WILL_DRAW);

		fTop = new AccountView("top");
		if (account)
			fSettings->Load(account, fTop);
		else
			fSettings->LoadTemplate(fTop);

		BButton* cancel = new BButton("Cancel", new BMessage(kCancel));
		BButton* ok = new BButton("OK", new BMessage(kOK));

		const float spacing = be_control_look->DefaultItemSpacing();

		SetLayout(new BGroupLayout(B_VERTICAL, spacing));
		AddChild(BGroupLayoutBuilder(B_VERTICAL, spacing)
			.Add(fAccountName)
			.Add(divider)
			.Add(fTop)
			.AddGroup(B_HORIZONTAL, spacing)
				.AddGlue()
				.Add(cancel)
				.Add(ok)
			.End()
			.AddGlue()
			.SetInsets(spacing, spacing, spacing, 0)
		);

		CenterOnScreen();
	}

	void MessageReceived(BMessage* msg)
	{
		switch (msg->what) {
			case kOK:
				if (fSettings->Save(fAccountName->Text(), fTop) == B_OK)
					Close();
// TODO: Error!
				break;
			case kCancel:
				Close();
				break;
			case kChanged:
				msg->PrintToStream();
				break;
			default:
				BWindow::MessageReceived(msg);
		}
	}

private:
	ProtocolSettings*	fSettings;
	AccountView*		fTop;
	BTextControl*		fAccountName;
};


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
