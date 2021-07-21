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
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <MenuField.h>
#include <Notification.h>
#include <ScrollView.h>
#include <SeparatorView.h>

#include "AccountsMenu.h"
#include "AppMessages.h"
#include "AppPreferences.h"
#include "ChatProtocolMessages.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "RosterView.h"
#include "TemplateWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RosterEditWindow"


const uint32 kSendMessage = 'RWSM';
const uint32 kAddMember = 'RWAM';
const uint32 kRemoveMember = 'RWRM';
const uint32 kEditMember = 'RWEM';
const uint32 kSelAccount = 'RWSA';
const uint32 kSelNoAccount = 'RWNA';

RosterEditWindow* RosterEditWindow::fInstance = NULL;


RosterEditWindow::RosterEditWindow(Server* server)
	:
	BWindow(BRect(0, 0, 300, 400), B_TRANSLATE("Roster"), B_FLOATING_WINDOW, 0),
	fServer(server),
	fEditingWindow(NULL)
{
	fRosterView = new RosterView("buddyView", server);
	fRosterView->SetInvocationMessage(new BMessage(kEditMember));

	fAccountField = new BMenuField("accountMenuField", NULL,
		new AccountsMenu("accountMenu", BMessage(kSelAccount),
			new BMessage(kSelNoAccount)));

	font_height fontHeight;
	fRosterView->GetFontHeight(&fontHeight);
	int16 buttonHeight = int16(fontHeight.ascent + fontHeight.descent + 12);
	BSize charButtonSize(buttonHeight, buttonHeight);
	BButton* fAddButton = new BButton("+", new BMessage(kAddMember));
	BButton* fRemoveButton = new BButton("-", new BMessage(kRemoveMember));
	fAddButton->SetExplicitSize(charButtonSize);
	fRemoveButton->SetExplicitSize(charButtonSize);

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


RosterEditWindow::~RosterEditWindow()
{
	fInstance = NULL;
}


RosterEditWindow*
RosterEditWindow::Get(Server* server)
{
	if (fInstance == NULL) {
		fInstance = new RosterEditWindow(server);
	}
	return fInstance;
}


bool
RosterEditWindow::Check()
{
	return (fInstance != NULL);
}


void
RosterEditWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kEditMember:
		{
			if (fEditingWindow != NULL && fEditingWindow->Lock()) {
				fEditingWindow->Quit();
				fEditingUser.SetTo("");
			}

			int index = message->FindInt32("index");
			RosterItem* ritem = fRosterView->ListView()->RosterItemAt(index);
			if (ritem == NULL)
				return;

			User* user = ritem->GetContact();
			fEditingUser.SetTo(user->GetId().String());

			// The response IM_EXTENDED_CONTACT_INFO is used to populate the
			// TemplateWindow.
			BMessage* request = new BMessage(IM_MESSAGE);
			request->AddInt32("im_what", IM_GET_EXTENDED_CONTACT_INFO);
			request->AddString("user_id", user->GetId());
			user->GetProtocolLooper()->PostMessage(request);

			BMessage* edit = new BMessage(IM_MESSAGE);
			edit->AddInt32("im_what", IM_CONTACT_LIST_EDIT_CONTACT);

			fEditingWindow =
				new TemplateWindow(B_TRANSLATE("Editing contact"), "roster",
					edit, fServer, user->GetProtocolLooper()->GetInstance());
			fEditingWindow->Show();
			break;
		}
		case kAddMember:
		{
			BMessage* add = new BMessage(IM_MESSAGE);
			add->AddInt32("im_what", IM_CONTACT_LIST_ADD_CONTACT);
			TemplateWindow* win =
				new TemplateWindow(B_TRANSLATE("Adding contact"), "roster",
					add, fServer);
			win->Show();
			break;
		}
		case kRemoveMember:
		{
			int index = message->FindInt32("index");
			RosterItem* ritem = fRosterView->ListView()->RosterItemAt(index);
			if (ritem == NULL)
				return;
			User* user = ritem->GetContact();

			BMessage* rem = new BMessage(IM_MESSAGE);
			rem->AddInt32("im_what", IM_CONTACT_LIST_REMOVE_CONTACT);
			rem->AddString("user_id", user->GetId());

			user->GetProtocolLooper()->PostMessage(rem);
			break;
		}
		case IM_MESSAGE: {
			if (message->GetInt32("im_what", 0) == IM_EXTENDED_CONTACT_INFO)
				if (message->GetString("user_id", "") == fEditingUser)
					fEditingWindow->PostMessage(message);
			fRosterView->MessageReceived(message);
			break;
		}
		case kSelAccount:
		{
			AccountInstances accounts = fServer->GetActiveAccounts();

			int index = message->FindInt32("index") - 1;
			if (index < 0 || index > (accounts.CountItems() - 1))
				return;
			fRosterView->SetAccount(accounts.ValueAt(index));
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
