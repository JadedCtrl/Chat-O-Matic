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

#include "CayaMessages.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "CayaUtils.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "RosterView.h"


const uint32 kSendMessage = 'RWSM';
const uint32 kSelAccount = 'RWSA';
const uint32 kSelNoAccount = 'RWNA';


RosterWindow::RosterWindow(const char* title, BMessage* selectMsg,
	BMessenger* messenger, Server* server, bigtime_t instance)
	:
	BWindow(BRect(0, 0, 300, 400), title, B_FLOATING_WINDOW, 0),
	fTarget(messenger),
	fMessage(selectMsg),
	fAccounts(server->GetAccounts()),
	fServer(server)
{
	fRosterView = new RosterView("buddyView", server, instance),
	fRosterView->SetInvocationMessage(new BMessage(kSendMessage));

	fOkButton = new BButton("OK", new BMessage(kSendMessage));
	fAccountField = new BMenuField("accountMenuField", NULL,
		CreateAccountMenu(fAccounts, BMessage(kSelAccount),
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

			if (ritem == NULL)
				return;

			User* user = ritem->GetContact();
			fMessage->AddString("user_id", user->GetId());
			fMessage->AddInt64("instance", user->GetProtocolLooper()->GetInstance());
			fTarget->SendMessage(fMessage);
			PostMessage(B_QUIT_REQUESTED);
			break;
		}
		case kSelAccount:
		{
			int index = message->FindInt32("index") - 1;
			if (index < 0 || index > (fAccounts.CountItems() - 1))
				return;
			fRosterView->SetAccount(fAccounts.ValueAt(index));
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
