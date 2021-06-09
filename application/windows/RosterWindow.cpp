/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
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
#include "SearchBarTextControl.h"
#include "Server.h"


const uint32 kSearchContact = 'RWSC';
const uint32 kSendMessage = 'RWSM';


RosterWindow::RosterWindow(const char* title, BMessage* selectMsg,
	BMessenger* messenger, Server* server)
	:
	BWindow(BRect(0, 0, 300, 400), title, B_FLOATING_WINDOW, 0),
	fTarget(messenger),
	fMessage(selectMsg),
	fServer(server)
{
	SearchBarTextControl* searchBox = 
		new SearchBarTextControl(new BMessage(kSearchContact));

	fListView = new RosterListView("buddyView");
	fListView->SetInvocationMessage(new BMessage(kSendMessage));
	BScrollView* scrollView = new BScrollView("scrollview", fListView,
		B_WILL_DRAW, false, true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.AddGroup(B_VERTICAL)
			.SetInsets(5, 5, 5, 10)
			.Add(searchBox)
			.Add(scrollView)
		.End()
	.End();

	_PopulateRosterList();

	CenterOnScreen();
}


void
RosterWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kSearchContact:
		{
			void* control = NULL;
			if (message->FindPointer("source", &control) != B_OK)
				return;

			SearchBarTextControl* searchBox 
				= static_cast<SearchBarTextControl*>(control);
			if (searchBox == NULL)
				return;

			RosterMap map = fServer->Contacts();
			for (uint32 i = 0; i < map.CountItems(); i++) {
				Contact* linker = map.ValueAt(i);
				RosterItem* item = linker->GetRosterItem();

				// If the search filter has been deleted show all the items,
				// otherwise remove the item in order to show only items
				// that matches the search criteria
				if (strcmp(searchBox->Text(), "") == 0)
					AddItem(item);
				else if (linker->GetName().IFindFirst(searchBox->Text()) == B_ERROR)
					RemoveItem(item);
				else
					AddItem(item);
				UpdateListItem(item);
			}
			break;
		}

		case kSendMessage:
		{
			int index = message->FindInt32("index");
			RosterItem* ritem = ItemAt(index);

			if (ritem == NULL)
				return;

			User* user = ritem->GetContact();
			fMessage->AddString("user_id", user->GetId());
			fTarget->SendMessage(fMessage);
			PostMessage(B_QUIT_REQUESTED);

			break;
		}

		case IM_MESSAGE:
			ImMessage(message);
			break;

		default:
			BWindow::MessageReceived(message);
	}
}


void
RosterWindow::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");
	switch (im_what) {
		case IM_STATUS_SET:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return;

			RosterItem*	rosterItem = fServer->ContactById(msg->FindString("user_id"))->GetRosterItem();

			if (rosterItem) {
				UpdateListItem(rosterItem);

				// Add or remove item
				switch (status) {
					/*case CAYA_OFFLINE:
						// By default offline contacts are hidden
						if (!CayaPreferences::Item()->HideOffline)
							break;
						if (HasItem(rosterItem))
							RemoveItem(rosterItem);
						return;*/
					default:
						// Add item because it has a non-offline status
						if (!HasItem(rosterItem))
							AddItem(rosterItem);
						break;
				}

				UpdateListItem(rosterItem);

				// Sort list view again
				fListView->Sort();

				// Check if the user want the notification
				if (!CayaPreferences::Item()->NotifyContactStatus)
					break;

				switch (status) {
					case CAYA_ONLINE:
					case CAYA_OFFLINE:
						// Notify when contact is online or offline
						if (status == CAYA_ONLINE) {
							BString message;
							message << rosterItem->GetContact()->GetName();

							if (status == CAYA_ONLINE)
								message << " is available!";
							else
								message << " is offline!";

							BNotification notification(B_INFORMATION_NOTIFICATION);
							notification.SetGroup(BString("Caya"));
							notification.SetTitle(BString("Presence"));
							notification.SetIcon(rosterItem->Bitmap());
							notification.SetContent(message);
							notification.Send();
						}
						break;
					default:
						break;
				}
			}
			break;
		}
		case IM_AVATAR_SET:
		case IM_CONTACT_INFO:
		case IM_EXTENDED_CONTACT_INFO:
		{
			RosterItem*	rosterItem
				= fServer->ContactById(msg->FindString("user_id"))->GetRosterItem();
			if (rosterItem)
				UpdateListItem(rosterItem);
			break;
		}
	}
}


int32
RosterWindow::CountItems() const
{
	return fListView->CountItems();
}


RosterItem*
RosterWindow::ItemAt(int index)
{
	return dynamic_cast<RosterItem*>(fListView->ItemAt(index));
}


void
RosterWindow::AddItem(RosterItem* item)
{
	// Don't add offline items and avoid duplicates
	if ((item->Status() == CAYA_OFFLINE) 
		&& CayaPreferences::Item()->HideOffline)
		return;
	
	if (HasItem(item))
		return;

	// Add item and sort
	fListView->AddItem(item);
	fListView->Sort();
}


bool
RosterWindow::HasItem(RosterItem* item)
{
	return fListView->HasItem(item);
}


void
RosterWindow::RemoveItem(RosterItem* item)
{
	// Remove item and sort
	fListView->RemoveItem(item);
	fListView->Sort();
}


void
RosterWindow::UpdateListItem(RosterItem* item)
{
	if (fListView->HasItem(item))
		fListView->InvalidateItem(fListView->IndexOf(item));
}


void
RosterWindow::_PopulateRosterList()
{
	RosterMap contacts = fServer->Contacts();

	for (int i = 0; i < contacts.CountItems(); i++)
		AddItem(contacts.ValueAt(i)->GetRosterItem());
}


