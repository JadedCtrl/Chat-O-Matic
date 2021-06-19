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

#include "RosterEditWindow.h"

#include <Button.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Notification.h>
#include <ScrollView.h>
#include <SeparatorView.h>

#include "CayaMessages.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "CayaUtils.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "RosterView.h"
#include "TemplateWindow.h"


const uint32 kSendMessage = 'RWSM';
const uint32 kAddMember = 'RWAM';
const uint32 kRemoveMember = 'RWRM';
const uint32 kEditMember = 'RWEM';
const uint32 kSelAccount = 'RWSA';
const uint32 kSelNoAccount = 'RWNA';


RosterEditWindow::RosterEditWindow(Server* server)
	:
	BWindow(BRect(0, 0, 300, 400), "Roster", B_FLOATING_WINDOW, 0),
	fAccounts(server->GetAccounts()),
	fServer(server)
{
	fRosterView = new RosterView("buddyView", server);
	fRosterView->SetInvocationMessage(new BMessage(kEditMember));

	fAccountField = new BMenuField("accountMenuField", NULL,
		CreateAccountMenu(fAccounts, BMessage(kSelAccount),
			new BMessage(kSelNoAccount)));

	font_height fontHeight;
	fRosterView->GetFontHeight(&fontHeight);
	int16 buttonHeight = int16(fontHeight.ascent + fontHeight.descent + 12);
	BSize charButtonSize(buttonHeight, buttonHeight);
	BButton* fAddButton = new BButton("+", new BMessage(kAddMember));
	BButton* fRemoveButton = new BButton("-", new BMessage(kRemoveMember));
	fAddButton->SetExplicitSize(charButtonSize);
	fAddButton->SetEnabled(true);
	fRemoveButton->SetExplicitSize(charButtonSize);
	fRemoveButton->SetEnabled(false);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fRosterView)
		.AddGroup(B_HORIZONTAL, 0, 0.0)
			.Add(fAccountField)
			.AddGlue()

			.Add(new BSeparatorView(B_VERTICAL))
			.AddGroup(B_VERTICAL, 0, 0.0)
				.Add(new BSeparatorView(B_HORIZONTAL))
				.AddGroup(B_HORIZONTAL, 1, 0.0)
					.SetInsets(1)
					.Add(fRemoveButton)
					.Add(fAddButton)
				.End()
				.Add(new BSeparatorView(B_HORIZONTAL))
			.End()
			.Add(new BSeparatorView(B_VERTICAL))
		.End()
	.End();
	CenterOnScreen();
}


void
RosterEditWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kEditMember:
		{
			int index = message->FindInt32("index");
			RosterItem* ritem = fRosterView->ListView()->RosterItemAt(index);
			if (ritem == NULL)
				return;
			User* user = ritem->GetContact();

			TemplateWindow* win =
				new TemplateWindow("Editing contact", "roster", new BMessage(),
					fServer, user->GetProtocolLooper()->GetInstance());
			win->Show();
			break;
		}
		case kAddMember:
		{
			BMessage* add = new BMessage(IM_MESSAGE);
			add->AddInt32("im_what", IM_CONTACT_LIST_ADD_CONTACT);
			TemplateWindow* win =
				new TemplateWindow("Adding contact", "roster", add, fServer);
			win->Show();
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
		default:
			BWindow::MessageReceived(message);
	}
}


void
RosterEditWindow::UpdateListItem(RosterItem* item)
{
	fRosterView->UpdateListItem(item);
}
