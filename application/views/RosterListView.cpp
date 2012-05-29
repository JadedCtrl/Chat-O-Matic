/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "RosterListView.h"

#include <Looper.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <SeparatorItem.h>

#include <string.h>
#include <stdio.h>

#include "ContactInfoWindow.h"
#include "ContactLinker.h"
#include "RosterItem.h"

const int32 kGetInfo	= 'GINF';
const int32 kShowLogs	= 'SHLG';
const int32 kAddPeople	= 'ADPL';


static int
compare_by_name(const void* _item1, const void* _item2)
{
	RosterItem* item1 = *(RosterItem**)_item1;
	RosterItem* item2 = *(RosterItem**)_item2;

	return strcasecmp(item1->GetContactLinker()->GetName().String(),
		item2->GetContactLinker()->GetName().String());
}


static int
compare_by_status(const void* _item1, const void* _item2)
{
	RosterItem* item1 = *(RosterItem**)_item1;
	RosterItem* item2 = *(RosterItem**)_item2;

	if (item1->Status() < item2->Status())
		return 1;
	if (item1->Status() > item2->Status())
		return 2;
	return 0;
}


RosterListView::RosterListView(const char* name)
	: BOutlineListView(name, B_SINGLE_SELECTION_LIST,
		B_WILL_DRAW | B_FRAME_EVENTS |
		B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE),
	fPrevItem(NULL)
{
	// Context menu
	fPopUp = new BPopUpMenu("contextMenu", false, false);
	fPopUp->AddItem(new BMenuItem("Get Informations", new BMessage(kGetInfo)));
	fPopUp->AddItem(new BMenuItem("Show Logs", new BMessage(kShowLogs)));
	fPopUp->AddItem(new BSeparatorItem());
	fPopUp->AddItem(new BMenuItem("Add to Address Book",
		new BMessage(kAddPeople)));
	fPopUp->SetTargetForItems(this);
}


//	#pragama mark -


void
RosterListView::AttachedToWindow()
{
	fPopUp->SetTargetForItems(this);
	SetTarget(this);
}


void
RosterListView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kGetInfo:
		{
			BPoint where;
			uint32 buttons;
			GetMouse(&where, &buttons);
			BListItem* item = ItemAt(IndexOf(where));
			RosterItem* ritem = reinterpret_cast<RosterItem*>(item);

			if (ritem == NULL)
				return;

			_InfoWindow(ritem->GetContactLinker());
			break;
		}
		default:
			BListView::MessageReceived(msg);
	}
}


void
RosterListView::MouseMoved(BPoint where, uint32 code, const BMessage* msg)
{
	BListView::MouseMoved(where, code, msg);
	return;

	switch (code) {
		case B_INSIDE_VIEW:
		{
			// Mouse cursor is inside this view, hide last item's popup
			// and show current item's popup
			BListItem* item = ItemAt(IndexOf(where));
			RosterItem* ritem = reinterpret_cast<RosterItem*>(item);

			if (ritem == NULL)
				return;

			// Hide previous item's popup
			if ((fPrevItem != NULL) && (fPrevItem != ritem))
				fPrevItem->GetContactLinker()->HidePopUp();

			// Show current item's popup
			ritem->GetContactLinker()->ShowPopUp(ConvertToScreen(where));

			// This will be the previous item
			fPrevItem = ritem;
			break;
		}
		case B_EXITED_VIEW:
			// Mouse cursor leaved this view, hide last item's popup
			if (fPrevItem != NULL)
				fPrevItem->GetContactLinker()->HidePopUp();
			break;
	}
}


void
RosterListView::MouseDown(BPoint where)
{
	BMessage* message = Looper()->CurrentMessage();

	int32 buttons = 0;
	(void)message->FindInt32("buttons", &buttons);

	if (buttons == B_SECONDARY_MOUSE_BUTTON) {
		int32 index = IndexOf(where);
		if (index >= 0) {
			// Select list item
			Select(index);

			// Show context menu if right button is clicked
			(void)fPopUp->Go(ConvertToScreen(where), true, true, false);
		}
	} else {
		// Call original MouseDown()
		BListView::MouseDown(where);
	}
}


void
RosterListView::Draw(BRect updateRect)
{
	int32 count = CountItems();
	if (count == 0)
		return;

	BRect itemFrame(0, 0, Bounds().right, -1);
	for (int32 i = 0; i < count; i++) {
		BListItem* item = ItemAt(i);
		RosterItem* rosterItem = reinterpret_cast<RosterItem*>(item);

		if (!rosterItem->IsVisible())
			continue;

		itemFrame.bottom = itemFrame.top + ceilf(item->Height()) - 1;

		if (itemFrame.Intersects(updateRect))
			rosterItem->DrawItem(this, itemFrame);

		itemFrame.top = itemFrame.bottom + 1;
	}
}


void
RosterListView::Sort()
{
	SortItems(compare_by_name);
}


void
RosterListView::_InfoWindow(ContactLinker* linker)
{
	ContactInfoWindow* win = new ContactInfoWindow(linker);
	win->Show();
}
