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

#include <LayoutBuilder.h>
#include <Notification.h>
#include <ScrollView.h>

#include "CayaMessages.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "RosterView.h"
#include "Server.h"


const uint32 kSendMessage = 'RWSM';


RosterWindow::RosterWindow(const char* title, BMessage* selectMsg,
	BMessenger* messenger, Server* server)
	:
	BWindow(BRect(0, 0, 300, 400), title, B_FLOATING_WINDOW, 0),
	fTarget(messenger),
	fMessage(selectMsg),
	fServer(server)
{
	fRosterView = new RosterView("buddyView", server);
	fRosterView->SetInvocationMessage(new BMessage(kSendMessage));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.AddGroup(B_VERTICAL)
			.SetInsets(5, 5, 5, 10)
			.Add(fRosterView)
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
