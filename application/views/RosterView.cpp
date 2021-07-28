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

#include "RosterView.h"

#include <Catalog.h>
#include <LayoutBuilder.h>
#include <Notification.h>
#include <ScrollView.h>
#include <StringItem.h>

#include "AppMessages.h"
#include "AppPreferences.h"
#include "Cardie.h"
#include "ChatProtocolMessages.h"
#include "RosterItem.h"
#include "RosterListView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "RosterView"


const uint32 kSearchContact = 'RWSC';


RosterView::RosterView(const char* title, Server* server, bigtime_t account)
	:
	BGroupView(title, B_VERTICAL, B_USE_DEFAULT_SPACING),
	fAccount(-1),
	fServer(server),
	fManualItem(new BStringItem("")),
	fManualStr("Select user %user%" B_UTF8_ELLIPSIS)
{
	fSearchBox = new BTextControl("searchBox", "", "",
		new BMessage(kSearchContact));
	fSearchBox->SetModificationMessage(new BMessage(kSearchContact));
		
	fListView = new RosterListView("buddyView");
	BScrollView* scrollView = new BScrollView("scrollview", fListView,
		B_WILL_DRAW, false, true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0.0f)
		.AddGroup(B_VERTICAL)
			.SetInsets(5, 5, 5, 10)
			.Add(fSearchBox)
			.Add(scrollView)
		.End()
	.End();

	SetAccount(account);
}


void
RosterView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case kSearchContact:
		{
			RosterMap map = _RosterMap();
			for (uint32 i = 0; i < map.CountItems(); i++) {
				Contact* linker = map.ValueAt(i);
				RosterItem* item = linker->GetRosterItem();

				// If the search filter has been deleted show all the items,
				// otherwise remove the item in order to show only items
				// that matches the search criteria
				if (strcmp(fSearchBox->Text(), "") == 0)
					fListView->AddItem(item);
				else if (linker->GetName().IFindFirst(fSearchBox->Text()) == B_ERROR)
					fListView->RemoveItem(item);
				else
					fListView->AddItem(item);
				UpdateListItem(item);
			}

			// If view has specific account selected, we want the user to be
			// able to select non-contacts of that protocol
			if (fAccount != - 1 && strcmp(fSearchBox->Text(), "") != 0) {
				BString label = fManualStr;
				label.ReplaceAll("%user%", fSearchBox->Text());

				fManualItem->SetText(label.String());
				fListView->AddItem(fManualItem);
			}
			else if (fListView->HasItem(fManualItem))
				fListView->RemoveItem(fManualItem);
			break;
		}
		case IM_MESSAGE:
			ImMessage(message);
			break;

		default:
			BGroupView::MessageReceived(message);
	}
}


void
RosterView::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");
	switch (im_what) {
		case IM_STATUS_SET:
		{
			int32 status;
			int64 instance;
			BString user_id = msg->FindString("user_id");
			if (msg->FindInt32("status", &status) != B_OK
				|| msg->FindInt64("instance", &instance) != B_OK
				|| user_id.IsEmpty() == true)
				return;

			Contact* contact = fServer->ContactById(user_id, instance);
			if (contact == NULL)
				return;

			RosterItem*	rosterItem = contact->GetRosterItem();

			if (rosterItem) {
				UpdateListItem(rosterItem);

				// Add or remove item
				switch (status) {
					/*case STATUS_OFFLINE:
						// By default offline contacts are hidden
						if (!AppPreferences::Item()->HideOffline)
							break;
						if (HasItem(rosterItem))
							RemoveItem(rosterItem);
						return;*/
					default:
						// Add item because it has a non-offline status
						fListView->AddItem(rosterItem);
						break;
				}

				UpdateListItem(rosterItem);

				// Check if the user want the notification
				if (!AppPreferences::Item()->NotifyContactStatus)
					break;

				switch (status) {
					case STATUS_ONLINE:
					case STATUS_OFFLINE:
						// Notify when contact is online or offline
						if (status == STATUS_ONLINE) {
							BString message;
							message << rosterItem->GetContact()->GetName();

							if (status == STATUS_ONLINE)
								message.SetTo(B_TRANSLATE("%name% is available!"));
							else
								message.SetTo(B_TRANSLATE("%name% is offline!"));
							message.ReplaceAll("%name%",
								rosterItem->GetContact()->GetName());

							BNotification notification(B_INFORMATION_NOTIFICATION);
							notification.SetGroup(BString(APP_NAME));
							notification.SetTitle(BString(B_TRANSLATE("Presence")));
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
		case IM_CONTACT_LIST_CONTACT_REMOVED:
		{
			int32 status = -1;
			int64 instance;
			BString user_id = msg->FindString("user_id");
			if (msg->FindInt32("status", &status) != B_OK
				|| msg->FindInt64("instance", &instance) != B_OK
				|| user_id.IsEmpty() == true)
				return;
			Contact* contact = fServer->ContactById(user_id, instance);
			if (contact == NULL)
				return;
			RosterItem*	rosterItem = contact->GetRosterItem();
			if (rosterItem)
				fListView->RemoveItem(rosterItem);
		}
		case IM_AVATAR_SET:
		case IM_CONTACT_INFO:
		case IM_EXTENDED_CONTACT_INFO:
		{
			int32 status = -1;
			int64 instance;
			BString user_id = msg->FindString("user_id");
			if (msg->FindInt32("status", &status) != B_OK
				|| msg->FindInt64("instance", &instance) != B_OK
				|| user_id.IsEmpty() == true)
				return;

			Contact* contact = fServer->ContactById(user_id, instance);
			if (contact == NULL)
				return;

			RosterItem*	rosterItem = contact->GetRosterItem();
			if (rosterItem)
				UpdateListItem(rosterItem);
			break;
		}
	}
}


void
RosterView::AttachedToWindow()
{
	fSearchBox->SetTarget(this);
	fSearchBox->MakeFocus(true);
}


void
RosterView::SetInvocationMessage(BMessage* msg)
{
	fListView->SetInvocationMessage(msg);
}


void
RosterView::SetAccount(bigtime_t instance_id)
{
	fAccount = instance_id;
	RosterMap contacts = _RosterMap();

	fListView->MakeEmpty();
	for (int i = 0; i < contacts.CountItems(); i++)
		fListView->AddItem(contacts.ValueAt(i)->GetRosterItem());
}


void
RosterView::UpdateListItem(RosterItem* item)
{
	if (fListView->HasItem(item))
		fListView->InvalidateItem(fListView->IndexOf(item));
}


RosterListView*
RosterView::ListView()
{
	return fListView;
}


RosterMap
RosterView::_RosterMap()
{
	RosterMap contacts;
	if (fAccount < 0)
		contacts = fServer->Contacts();
	else {
		ProtocolLooper* looper = fServer->GetProtocolLooper(fAccount);
		contacts = looper->Contacts();
	}
	return contacts;
}
