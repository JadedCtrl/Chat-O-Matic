/*
 * Copyright 2011-2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Dario Casalinuovo
 */

#include "ReplicantStatusView.h"

#include <stdio.h>

#include <Application.h>
#include <AppFileInfo.h>
#include <Bitmap.h>
#include <Catalog.h>
#include <Deskbar.h>
#include <IconUtils.h>
#include <Message.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <Window.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/BitmapUtils.h>
#include <libinterface/BitmapView.h>

#include "AppMessages.h"
#include "AppPreferences.h"
#include "Cardie.h"
#include "ChatProtocolMessages.h"
#include "ReplicantMenuItem.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ReplicantStatusView"


extern "C" _EXPORT BView *instantiate_deskbar_item(void);


// The following handler is added to the Deskbar's looper
// to receive notifications from Caya
class ReplicantHandler : public BHandler {
public:
				 ReplicantHandler(const char* name, ReplicantStatusView* target)
				 	:
				 	BHandler(name)
				 	{
				 		fTarget = target;
				 	}

				 ~ReplicantHandler() {}

	virtual void MessageReceived(BMessage* message)
	{
		switch (message->what) {
			case IM_OWN_STATUS_SET:
			{
				int32 status;

				if (message->FindInt32("status", &status) != B_OK)
					return;
				fTarget->SetStatus((UserStatus)status);
				break;
			}
			default:
				BHandler::MessageReceived(message);
		}
	}
private:
	ReplicantStatusView* fTarget;
};


ReplicantStatusView::ReplicantStatusView()
	:
	BView(BRect(0, 0, 15, 15), "ReplicantStatusView", 	
		B_FOLLOW_LEFT | B_FOLLOW_TOP, B_WILL_DRAW)
{
	_Init();
}


ReplicantStatusView::ReplicantStatusView(BMessage* archive)
	:
	BView(archive)
{
	_Init();
}


ReplicantStatusView::~ReplicantStatusView()
{
	delete fCayaMsg;
	delete fReplicantHandler;
	delete fReplicantMenu;

	// TODO: Use a list for that
	// maybe our List wrapper to std::list
	delete fConnectingIcon;
	delete fIcon;
	delete fOfflineIcon;
	delete fBusyIcon;
	delete fAwayIcon;
}


void
ReplicantStatusView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case APP_REPLICANT_STATUS_SET:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return;

			SetStatus((UserStatus)status);
			fCayaMsg->SendMessage(msg);
			break;
		}
		case APP_REPLICANT_EXIT:
		case APP_SHOW_SETTINGS:
		case APP_REPLICANT_SHOW_WINDOW:
		case APP_REPLICANT_MESSENGER:
			fCayaMsg->SendMessage(msg);
			break;
		default:
			BView::MessageReceived(msg);
	}
}


void
ReplicantStatusView::SetStatus(UserStatus status)
{
	for (int32 i = 0; i < fReplicantMenu->CountItems(); i++) {
		ReplicantMenuItem* item
			= dynamic_cast<ReplicantMenuItem*>(fReplicantMenu->ItemAt(i));
		if (item == NULL)
			continue;

		if (item->IsMarked())
			item->SetMarked(false);

		if (item && item->Status() == status && !item->IsCustom())
			item->SetMarked(true);

	}

	switch (status) {
		case STATUS_AWAY:
			fIcon = fAwayIcon;
		break;
		case STATUS_DO_NOT_DISTURB:
			fIcon = fBusyIcon;
		break;
		case STATUS_CUSTOM_STATUS:
			fIcon = fAppIcon;
		break;
		case STATUS_INVISIBLE:
		case STATUS_OFFLINE:
			fIcon = fOfflineIcon;
		break;
		default:
			fIcon = fIcon;
		break;
	}
	Invalidate();
}



// Draw our deskbar icon.
void
ReplicantStatusView::Draw(BRect rect)
{
	SetDrawingMode(B_OP_ALPHA);
	DrawBitmap(fIcon);
}


ReplicantStatusView*
ReplicantStatusView::Instantiate(BMessage* archive)
{
	if (!validate_instantiation(archive, "ReplicantStatusView"))
		return NULL;

	return new ReplicantStatusView(archive);
}


status_t
ReplicantStatusView::Archive(BMessage* archive, bool deep) const
{
	status_t status = BView::Archive(archive, deep);

	if (status == B_OK)
		status = archive->AddString("add_on", APP_SIGNATURE);

	if (status == B_OK)
		status = archive->AddString("class", "ReplicantStatusView");

	return status;
}


void
ReplicantStatusView::AttachedToWindow()
{
	BView::AttachedToWindow();
	if (Parent())
		SetViewColor(Parent()->ViewColor());
	else
		SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	SetLowColor(ViewColor());

	fReplicantHandler = new ReplicantHandler("CayaReplicantHandler", this);
	if (Window()->Lock()) {
        Window()->AddHandler(fReplicantHandler);
        Window()->Unlock();
	}

	BMessage msg(APP_REPLICANT_MESSENGER);
	BMessenger messenger(fReplicantHandler);
	if (!messenger.IsValid())
		return;
	msg.AddMessenger("messenger", messenger);
	fCayaMsg->SendMessage(&msg);
}


void
ReplicantStatusView::DetachedFromWindow()
{
	if (Window()->Lock()) {
		Window()->RemoveHandler(fReplicantHandler);
		Window()->Unlock();
	}
}


void
ReplicantStatusView::MouseDown(BPoint point)
{

	uint32 buttons;
	if (LockLooper()) {
		GetMouse(&point, &buttons, false);
		UnlockLooper();
	}
	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		// Show / Hide Window command
		BMessage msg(APP_REPLICANT_SHOW_WINDOW);
		fCayaMsg->SendMessage(&msg);
	} else if(buttons & B_SECONDARY_MOUSE_BUTTON) {
		// Build replicant menu
		_ShowMenu(point);
	}
}


void
ReplicantStatusView::_Init()
{
	SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	// Creating the Send messenger and sending
	// a messenger targeting this to Caya.
	// This will allow the Replicant to communicate
	// whith Caya.
	fCayaMsg = new BMessenger(APP_SIGNATURE);

	fResources = ChatResources();

	//Get icons from resources
	fConnectingIcon = _GetIcon(kOnlineReplicant);
	fAppIcon = _GetIcon(kIconReplicant);
	fOfflineIcon = _GetIcon(kOfflineReplicant);
	fIcon = fOfflineIcon;
	fBusyIcon = _GetIcon(kBusyReplicant);
	fAwayIcon = _GetIcon(kAwayReplicant);
	fExitMenuIcon = _GetIcon(kExitMenuReplicant);
	fPreferencesIcon = _GetIcon(kToolIcon);

	// Build the replicant menu
	_BuildMenu();
}


BBitmap*
ReplicantStatusView::_GetIcon(const uint32 id)
{
	BBitmap* icon = IconFromResources(&fResources, id, B_MINI_ICON);
	return icon;
}


void
ReplicantStatusView::_BuildMenu()
{
	// Status menu

	fReplicantMenu = new BPopUpMenu(" -  ", false, false);
	// Add status menu items
	int32 s = STATUS_ONLINE;
	while (s >= STATUS_ONLINE && s < STATUS_STATUSES) {
		BMessage* msg = new BMessage(APP_REPLICANT_STATUS_SET);
		msg->AddInt32("status", s);

		ReplicantMenuItem* item = new ReplicantMenuItem(
			UserStatusToString((UserStatus)s), (UserStatus)s);
		fReplicantMenu->AddItem(item);

		// Mark offline status by default
		if (s == STATUS_OFFLINE)
			item->SetMarked(true);
		s++;
	}

	fReplicantMenu->AddItem(new BSeparatorItem());

	fReplicantMenu->AddItem(new BitmapMenuItem(B_TRANSLATE("Preferences "),
		new BMessage(APP_SHOW_SETTINGS), fPreferencesIcon));

	fReplicantMenu->AddItem(new BitmapMenuItem(B_TRANSLATE("Exit"),
		new BMessage(APP_REPLICANT_EXIT), fExitMenuIcon));

	fReplicantMenu->SetTargetForItems(this);
}


void
ReplicantStatusView::_ShowMenu(BPoint point)
{
	fReplicantMenu->SetTargetForItems(this);
	ConvertToScreen(&point);
	fReplicantMenu->Go(point, true, true, true);
}


extern "C" _EXPORT BView *
instantiate_deskbar_item(void)
{
	return new ReplicantStatusView();
}


// The following methods install
// and remove the Caya's replicant
// from Deskbar.
status_t
ReplicantStatusView::InstallReplicant()
{
	if (AppPreferences::Get()->DisableReplicant == true)
		return B_OK;
	
	BDeskbar deskbar;
	if (deskbar.HasItem("ReplicantStatusView")) {
		ReplicantStatusView::RemoveReplicant();
	}
	ReplicantStatusView* view = new ReplicantStatusView();
	return deskbar.AddItem(view);
}


status_t
ReplicantStatusView::RemoveReplicant()
{
	BDeskbar deskbar;
	return deskbar.RemoveItem("ReplicantStatusView");
}
