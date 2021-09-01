/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RoomListWindow.h"

#include <Button.h>
#include <Catalog.h>
#include <ColumnListView.h>
#include <ColumnTypes.h>
#include <LayoutBuilder.h>
#include <StringList.h>

#include "AccountsMenu.h"
#include "AppPreferences.h"
#include "ChatProtocolMessages.h"
#include "RoomListRow.h"
#include "Server.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Room directory"


const uint32 kSelectAcc = 'rlse';
const uint32 kSelectAll = 'rlsa';
const uint32 kJoinRoom = 'join';
RoomListWindow* RoomListWindow::fInstance = NULL;


RoomListWindow::RoomListWindow(Server* server)
	:
	BWindow(AppPreferences::Get()->RoomDirectoryRect,
		B_TRANSLATE("Room directory"), B_FLOATING_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	fServer(server),
	fAccount(-1)
{
	_InitInterface();
	CenterOnScreen();

	BMessage* request = new BMessage(IM_MESSAGE);
	request->AddInt32("im_what", IM_GET_ROOM_DIRECTORY);
	server->SendAllProtocolMessage(request);
}


RoomListWindow::~RoomListWindow()
{
	fInstance = NULL;
	AppPreferences::Get()->RoomDirectoryRect = Bounds();
	_EmptyList();

	for (int i = 0; i < fRows.CountItems(); i++) {
		BObjectList<RoomListRow>* list = fRows.ValueAt(i);
		if (list != NULL)
			delete list;
	}
}


RoomListWindow*
RoomListWindow::Get(Server* server)
{
	if (fInstance == NULL)
		fInstance = new RoomListWindow(server);
	return fInstance;
}


bool
RoomListWindow::Check()
{
	return (fInstance != NULL);
}


void
RoomListWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case IM_MESSAGE:
		{
			if (msg->GetInt32("im_what", -1) != IM_ROOM_DIRECTORY)
				break;

			int64 instance;
			BString id;
			if (msg->FindInt64("instance", &instance) == B_OK
					&& msg->FindString("chat_id", &id) == B_OK) {
				RoomListRow* row = new RoomListRow(msg);

				bool fnd = false;
				BObjectList<RoomListRow>* list = fRows.ValueFor(instance, &fnd);
				if (fnd == false || list == NULL) {
					list = new BObjectList<RoomListRow>(20, true);
					fRows.AddItem(instance, list);
				}
				list->AddItem(row);

				if (fAccount == -1 || instance == fAccount)
					fListView->AddRow(row);
			}
			break;
		}
		case kSelectAll:
		{
			_EmptyList();
			for (int i = 0; i < fRows.CountItems(); i++) {
				BObjectList<RoomListRow>* list = fRows.ValueAt(i);
				if (list != NULL)
					for (int j = 0; j < list->CountItems(); j++) {
						RoomListRow* row = list->ItemAt(j);
						if (row != NULL)
							fListView->AddRow(row);
					}
			}
			fAccount = -1;
			break;
		}
		case kSelectAcc:
		{
			msg->PrintToStream();
			int64 instance;
			if (msg->FindInt64("instance", &instance) == B_OK) {
				bool fnd = false;
				BObjectList<RoomListRow>* list = fRows.ValueFor(instance, &fnd);
				if (fnd == false || list == NULL)
					break;

				_EmptyList();
				for (int i = 0; i < list->CountItems(); i++) {
					RoomListRow* row = list->ItemAt(i);
					if (row != NULL)
						fListView->AddRow(row);
				}
				fAccount = instance;
			}
			break;
		}
		case kJoinRoom:
		{
			RoomListRow* row =
				(RoomListRow*)fListView->CurrentSelection();

			if (row != NULL) {
				BMessage* joinMsg = row->Message();
				joinMsg->ReplaceInt32("im_what", IM_JOIN_ROOM);
				fServer->SendProtocolMessage(joinMsg);
				Quit();
			}
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
RoomListWindow::_InitInterface()
{
	float size = BFont().Size();
	BStringColumn* name = new BStringColumn(B_TRANSLATE("Name"), 130, 50, 300,
		B_TRUNCATE_END);
	BStringColumn* desc = new BStringColumn(B_TRANSLATE("Description"), 270,
		50, 5000, B_TRUNCATE_END);
	BStringColumn* category = new BStringColumn("Category", 90, 50, 300,
		B_TRUNCATE_END);
	BIntegerColumn* users = new BIntegerColumn("Users", 70, 10, 300);

	fListView = new BColumnListView("roomList", B_NAVIGABLE, B_PLAIN_BORDER);	
	fListView->SetInvocationMessage(new BMessage(kJoinRoom));
	fListView->SetSelectionMode(B_SINGLE_SELECTION_LIST);
	fListView->AddColumn(name, kNameColumn);
	fListView->AddColumn(desc, kDescColumn);
	fListView->AddColumn(category, kCatColumn);
	fListView->AddColumn(users, kUserColumn);

	AccountsMenu* accsMenu = new AccountsMenu("accounts", BMessage(kSelectAcc),
		new BMessage(kSelectAll), fServer);
	BMenuField* accsField = new BMenuField(NULL, accsMenu);

	fJoinButton = new BButton("joinRoom", B_TRANSLATE("Join"),
		new BMessage(kJoinRoom));

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fListView)
		.AddGroup(B_HORIZONTAL)
			.Add(accsField)
			.AddGlue()
			.Add(fJoinButton)
		.End()
	.End();
}


void
RoomListWindow::_EmptyList()
{
	BRow* row = fListView->RowAt((int32)0, NULL);
	while (row != NULL) {
		fListView->RemoveRow(row);
		row = fListView->RowAt((int32)0, NULL);
	}
}
