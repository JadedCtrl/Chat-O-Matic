/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
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
#include <PopUpMenu.h>
#include <SpaceLayoutItem.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
#include <TranslationUtils.h>

#include <libinterface/BitmapUtils.h>
#include <libinterface/ToolButton.h>

#include "CayaConstants.h"
#include "CayaResources.h"
#include "CayaUtils.h"
#include "NotifyMessage.h"
#include "MainWindow.h"
#include "PreferencesDialog.h"
#include "RosterItem.h"
#include "RosterListView.h"
#include "Server.h"
#include "StatusView.h"

const int32 kLogin         = 'LOGI';
const int32 kSearchContact = 'SRCH';
const int32 kPreferences   = 'PRFS';


MainWindow::MainWindow() :
	BWindow(BRect(0, 0, 300, 400), "Caya", B_DOCUMENT_WINDOW, 0)
{	
	SetLayout(fStack = new BCardLayout());

	BBox* loginView = new BBox("loginView");
	
	BButton* login = new BButton(BRect(294.0, 302.0, 392.0, 328.0), "login",
		"Login", new BMessage(kLogin), B_FOLLOW_BOTTOM | B_FOLLOW_RIGHT);

	loginView->SetLayout(new BGroupLayout(B_HORIZONTAL));
	loginView->AddChild(BGroupLayoutBuilder(B_VERTICAL, 5)
		.Add(BGridLayoutBuilder(10, 10)
		.Add(new BStringView("label_u", "Username"), 0, 0)
		.Add(fUsername = new BTextControl("username", NULL, NULL, NULL), 1, 0)
		.Add(new BStringView("label_p", "Password"), 0, 1)
		.Add(fPassword = new BTextControl("password", NULL, NULL,
			new BMessage(kLogin)), 1, 1), 2)
		.Add(login, 3)				
	);

	fStack->AddView(loginView);

	fStatusView = new StatusView("statusView");

	BTextControl* searchBox = new BTextControl("searchBox", NULL, NULL,
		new BMessage(kSearchContact));

	BBox* rosterView = new BBox("rosterView");

	fListView = new RosterListView("buddyView");
	fListView->SetInvocationMessage(new BMessage(OPEN_WINDOW));
	BScrollView* scrollView = new BScrollView("scrollview", fListView,
		B_WILL_DRAW, false, true);

	// Wrench menu
	BPopUpMenu* wrenchMenu = new BPopUpMenu("Wrench");
	(void)wrenchMenu->AddItem(new BMenuItem("Preferences...",
		new BMessage(kPreferences)));
	wrenchMenu->SetTargetForItems(this);

	// Tool icon
	BResources* res = CayaResources();
	BBitmap* toolIcon = IconFromResources(res, kToolIcon);
	delete res;

	// Wrench tool button
	ToolButton* wrench = new ToolButton(NULL, NULL);
	wrench->SetBitmap(toolIcon);
	wrench->SetMenu(wrenchMenu);

	rosterView->SetLayout(new BGridLayout(5, 5));
	rosterView->AddChild(BGridLayoutBuilder(5, 0)
		.Add(fStatusView, 0, 0)
		.Add(wrench, 1, 0)
		.Add(searchBox, 0, 1)
		.Add(scrollView, 0, 2, 2)
		.SetInsets(5, 5, 5, 10)
	);

	fStack->AddView(rosterView);			
	fStack->SetVisibleItem((long)0);

	AddShortcut('a', B_COMMAND_KEY, new BMessage(B_ABOUT_REQUESTED));
	MoveTo(BAlert::AlertPosition(Bounds().Width(), Bounds().Height() / 2));

	fPassword->TextView()->HideTyping(true);
	fUsername->MakeFocus(true);

	fSrv = new Server(this);
	AddFilter(fSrv);

	CenterOnScreen();
}


bool
MainWindow::QuitRequested()
{
	fListView->MakeEmpty();
	fSrv->Quit();
	be_app->PostMessage(B_QUIT_REQUESTED);
	return true;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch(message->what) {
		case kLogin:
		{
			BString username(fUsername->Text());
			BString password(fPassword->Text());
			if (username == "" || password == "")
				return;

			BMessage config;
			config.AddString("username", username);
			config.AddString("password", password);
			config.AddString("resource", "caya");

			fSrv->UpdateSettings(config);
			fSrv->Login();

			fStack->SetVisibleItem((long)1);
			break;			
		}
		case kSearchContact:
		{
			void* control = NULL;
			if (message->FindPointer("source", &control) != B_OK)
				return;

			BTextControl* searchBox = static_cast<BTextControl*>(control);
			if (searchBox == NULL)
				return;

			RosterMap map = fSrv->RosterItems();
			for (uint32 i = 0; i < map.CountItems(); i++) {
				ContactLinker* linker = map.ValueAt(i);
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
		case kPreferences: {
			PreferencesDialog* dialog = new PreferencesDialog();
			dialog->Show();
			break;
		}
		case IM_MESSAGE:
			ImMessage(message);
			break;
		case IM_ERROR:
			ImError(message);
			break;
		default:
			BWindow::MessageReceived(message);
			break;
	}	
}

void
MainWindow::ImError(BMessage* msg)
{
	//FIXME: better error handling..
	BAlert* alert = new BAlert("Error", msg->FindString("error"), "Ouch!");
	alert->Go();
	fStack->SetVisibleItem((long)0);	
}

void
MainWindow::ImMessage(BMessage* msg)
{	
	int32 im_what = msg->FindInt32("im_what");
	switch(im_what) {
		case IM_OWN_CONTACT_INFO:
		{			
			fStatusView->SetName(msg->FindString("nick"));

			entry_ref ref;
			if (msg->FindRef("ref", &ref) == B_OK) {
				BBitmap* bitmap = BTranslationUtils::GetBitmap(&ref);
				fStatusView->SetAvatar(bitmap);
			}
			break;
		}
		case IM_STATUS_CHANGED:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return;

			RosterItem*	rosterItem = fSrv->RosterItemForId(msg->FindString("id"));

			if (rosterItem) {
				// Add or remove item
				UpdateListItem(rosterItem);
				switch (status) {
					case CAYA_OFFLINE:
						if (HasItem(rosterItem))
							RemoveItem(rosterItem);
						return;
					default:
						if (!HasItem(rosterItem))
							AddItem(rosterItem);
						break;
				}
				UpdateListItem(rosterItem);

				// Sort list view again
				fListView->Sort();
			}
			break;
		}
		case IM_AVATAR_CHANGED:
		case IM_CONTACT_INFO:
		{
			RosterItem*	rosterItem = fSrv->RosterItemForId(msg->FindString("id"));
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
	if ((item->Status() == CAYA_OFFLINE) || HasItem(item))
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
