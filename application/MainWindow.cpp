/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Application.h>
#include <Alert.h>
#include <Button.h>
#include <CardLayout.h>
#include <ListView.h>
#include <Box.h>
#include <CheckBox.h>
#include <Entry.h>
#include <GridLayout.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <MenuItem.h>
#include <Notification.h>
#include <PopUpMenu.h>
#include <SpaceLayoutItem.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
#include <TranslationUtils.h>

#include <libinterface/BitmapUtils.h>
#include <libinterface/ToolButton.h>

#include "AccountManager.h"
#include "CayaConstants.h"
#include "CayaMessages.h"
#include "CayaProtocolMessages.h"
#include "CayaPreferences.h"
#include "CayaResources.h"
#include "CayaUtils.h"
#include "NotifyMessage.h"
#include "MainWindow.h"
#include "PreferencesDialog.h"
#include "ReplicantStatusView.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "SearchBarTextControl.h"
#include "Server.h"
#include "StatusView.h"


const uint32 kLogin			= 'LOGI';
const uint32 kSearchContact = 'SRCH';


MainWindow::MainWindow()
	:
	BWindow(BRect(0, 0, 300, 400), "Caya", B_TITLED_WINDOW, 0),
	fWorkspaceChanged(false)
{
	fStatusView = new StatusView("statusView");

	SearchBarTextControl* searchBox = 
		new SearchBarTextControl(new BMessage(kSearchContact));

	fListView = new RosterListView("buddyView");
	fListView->SetInvocationMessage(new BMessage(CAYA_OPEN_CHAT_WINDOW));
	BScrollView* scrollView = new BScrollView("scrollview", fListView,
		B_WILL_DRAW, false, true);

	// Wrench menu
	BPopUpMenu* wrenchMenu = new BPopUpMenu("Wrench");
	(void)wrenchMenu->AddItem(new BMenuItem("About" B_UTF8_ELLIPSIS,
		new BMessage(B_ABOUT_REQUESTED)));
	(void)wrenchMenu->AddItem(new BSeparatorItem());
	(void)wrenchMenu->AddItem(new BMenuItem("Preferences" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_SHOW_SETTINGS)));
	(void)wrenchMenu->AddItem(new BSeparatorItem());
	(void)wrenchMenu->AddItem(new BMenuItem("Quit",
		new BMessage(B_QUIT_REQUESTED)));
	wrenchMenu->SetTargetForItems(this);

	// Tool icon
	BResources* res = CayaResources();
	BBitmap* toolIcon = IconFromResources(res, kToolIcon);
	delete res;

	// Wrench tool button
	ToolButton* wrench = new ToolButton(NULL, NULL);
	wrench->SetBitmap(toolIcon);
	wrench->SetMenu(wrenchMenu);

	SetLayout(new BGridLayout(1, 2));
	AddChild(BGridLayoutBuilder(1, 2)
		.Add(searchBox, 0, 0)
		.Add(wrench, 1, 0)
		.Add(scrollView, 0, 1, 2)
		.Add(fStatusView, 0, 2, 2)
		.SetInsets(5, 5, 5, 10)
	);

	AddShortcut('a', B_COMMAND_KEY, new BMessage(B_ABOUT_REQUESTED));
	MoveTo(BAlert::AlertPosition(Bounds().Width(), Bounds().Height() / 2));

	// Filter messages using Server
	fServer = new Server();
	AddFilter(fServer);

	CenterOnScreen();

	//TODO check for errors here
	ReplicantStatusView::InstallReplicant();
}


void
MainWindow::Start()
{
	// Login all accounts
	fServer->LoginAll();
}


bool
MainWindow::QuitRequested()
{
	int32 button_index = 0;
	if(!CayaPreferences::Item()->DisableQuitConfirm)
	{
		BAlert* alert = new BAlert("Closing", "Are you sure you wan to quit?", "Yes", "No", NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING, B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		button_index = alert->Go();
	}

	if(button_index == 0) {
		fListView->MakeEmpty();
		fServer->Quit();
		CayaPreferences::Get()->Save();
		ReplicantStatusView::RemoveReplicant();
		be_app->PostMessage(B_QUIT_REQUESTED);
		return true;
	} else {
		return false;
	}
}


void
MainWindow::MessageReceived(BMessage* message)
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

			RosterMap map = fServer->RosterItems();
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
		case CAYA_SHOW_SETTINGS:
		{
			PreferencesDialog* dialog = new PreferencesDialog();
			dialog->Show();
			break;
		}
		case CAYA_OPEN_CHAT_WINDOW:
		{
			int index = message->FindInt32("index");
			RosterItem* ritem = ItemAt(index);
			if (ritem != NULL)
				ritem->GetContact()->ShowWindow(false, true);
			break;
		}

		case CAYA_REPLICANT_STATUS_SET:
		{
			int32 status;
			message->FindInt32("status", &status);
			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetStatus((CayaStatus)status);
			break;
		}

		case CAYA_REPLICANT_SHOW_WINDOW:
		{
			if (LockLooper()) {
				SetWorkspaces(B_CURRENT_WORKSPACE);
				
				if ((IsMinimized() || IsHidden()) 
					|| fWorkspaceChanged) {
					Minimize(false);
					Show();
					fWorkspaceChanged = false;
				} else if ((!IsMinimized() || !IsHidden())
					|| (!fWorkspaceChanged)) {
					Minimize(true);
				}
				UnlockLooper();
			}
			break;
		}

		case IM_MESSAGE:
			ImMessage(message);
			break;
		case IM_ERROR:
			ImError(message);
			break;
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(message);
			break;

		default:
			BWindow::MessageReceived(message);
	}
}


void
MainWindow::ImError(BMessage* msg)
{
	const char* error = NULL;
	const char* detail = msg->FindString("detail");

	if (msg->FindString("error", &error) != B_OK)
		return;

	// Format error message
	BString errMsg(error);
	if (detail)
		errMsg << "\n" << detail;

	BAlert* alert = new BAlert("Error", errMsg.String(), "OK", NULL, NULL,
		B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();
}


void
MainWindow::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");
	switch (im_what) {
		case IM_OWN_CONTACT_INFO:
			fStatusView->SetName(msg->FindString("name"));
			break;
		case IM_OWN_AVATAR_SET:
		{
			entry_ref ref;

			if (msg->FindRef("ref", &ref) == B_OK) {
				BBitmap* bitmap = BTranslationUtils::GetBitmap(&ref);
				fStatusView->SetAvatarIcon(bitmap);
			}
			break;
		}
		case IM_STATUS_SET:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return;

			RosterItem*	rosterItem = fServer->RosterItemForId(msg->FindString("id"));

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
				= fServer->RosterItemForId(msg->FindString("id"));
			if (rosterItem)
				UpdateListItem(rosterItem);
			break;
		}
	}
}


void
MainWindow::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_ACCOUNT_STATUS:
			fStatusView->SetStatus((CayaStatus)val);
			break;
	}
}


void
MainWindow::UpdateListItem(RosterItem* item)
{
	if (fListView->HasItem(item))
		fListView->InvalidateItem(fListView->IndexOf(item));
}


int32
MainWindow::CountItems() const
{
	return fListView->CountItems();
}


RosterItem*
MainWindow::ItemAt(int index)
{
	return dynamic_cast<RosterItem*>(fListView->ItemAt(index));
}


void
MainWindow::AddItem(RosterItem* item)
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
MainWindow::HasItem(RosterItem* item)
{
	return fListView->HasItem(item);
}


void
MainWindow::RemoveItem(RosterItem* item)
{
	// Remove item and sort
	fListView->RemoveItem(item);
	fListView->Sort();
}


void
MainWindow::WorkspaceActivated(int32 workspace, bool active)
{
	if (active)
		fWorkspaceChanged = false;
	else
		fWorkspaceChanged = true;
}
