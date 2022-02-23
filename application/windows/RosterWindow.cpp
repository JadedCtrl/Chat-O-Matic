/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "RosterWindow.h"

#include <Button.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Notification.h>
#include <ScrollView.h>

#include "AccountsMenu.h"
#include "AppMessages.h"
#include "AppPreferences.h"
#include "ChatProtocolMessages.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "RosterView.h"
#include "Server.h"


const uint32 kSendMessage = 'RWSM';
const uint32 kSelAccount = 'RWSA';
const uint32 kSelNoAccount = 'RWNA';


RosterWindow::RosterWindow(const char* title, BMessage* selectMsg,
	BMessenger* messenger, bigtime_t instance)
	:
	BWindow(BRect(0, 0, 300, 400), title, B_FLOATING_WINDOW,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fTarget(messenger),
	fMessage(selectMsg)
{
	fRosterView = new RosterView("buddyView", instance);
	fRosterView->SetInvocationMessage(new BMessage(kSendMessage));

	fOkButton = new BButton("OK", new BMessage(kSendMessage));

	AccountInstances accounts = Server::Get()->GetActiveAccounts();

	// If a specific instance is given, disallow selecting other accounts
	// In fact, don't even bother populating with them
	if (instance > -1) {
		BMenu* accountMenu = new BMenu("accountMenu");

		BString name = "N/A";
		for (int i = 0; i < accounts.CountItems(); i++)
			if (accounts.ValueAt(i) == instance) {
				name = accounts.KeyAt(i);
				break;
			}
		accountMenu->AddItem(new BMenuItem(name.String(), NULL));
		accountMenu->SetLabelFromMarked(true);
		accountMenu->ItemAt(0)->SetMarked(true);
		accountMenu->SetEnabled(false);

		fAccountField = new BMenuField("accountMenuField", NULL, accountMenu);
	}
	else
		fAccountField = new BMenuField("accountMenuField", NULL,
			new AccountsMenu("accountMenu", BMessage(kSelAccount),
				new BMessage(kSelNoAccount)));


	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fRosterView)
		.AddGroup(B_HORIZONTAL)
			.Add(fAccountField)
			.AddGlue()
			.Add(new BButton("Cancel", new BMessage(B_QUIT_REQUESTED)))
			.Add(fOkButton)
		.End()
	.End();
	CenterOnScreen();
}


void
RosterWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kSendMessage:
		{
			int index = message->FindInt32("index");
			RosterItem* ritem = fRosterView->ListView()->RosterItemAt(index);
			const char* search = fRosterView->SearchBox()->Text();
			BString user_id;
			int64 instance;

			if (ritem != NULL) {
				User* user = ritem->GetContact();
				user_id = user->GetId();
				instance = user->GetProtocolLooper()->GetInstance();
			}
			else if (search != NULL) {
				user_id = search;
				instance = fRosterView->GetAccount();
			}
			else
				return;

			User* user = ritem->GetContact();
			fMessage->AddString("user_id", user_id);
			fMessage->AddInt64("instance", instance);
			fTarget->SendMessage(fMessage);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case kSelAccount:
		{
			AccountInstances accounts = Server::Get()->GetActiveAccounts();

			int index = message->FindInt32("index") - 1;
			if (index < 0 || index > (accounts.CountItems() - 1))
				return;
			fRosterView->SetAccount(accounts.ValueAt(index));
			break;
		}
		case kSelNoAccount:
			fRosterView->SetAccount(-1);
			break;
		case IM_MESSAGE:
			fRosterView->MessageReceived(message);
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


void
RosterWindow::UpdateListItem(RosterItem* item)
{
	fRosterView->UpdateListItem(item);
}
