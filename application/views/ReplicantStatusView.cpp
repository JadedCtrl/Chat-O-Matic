/*
 * Copyright 2011, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Dario Casalinuovo
 */

#include <Application.h>
#include <AppFileInfo.h>
#include <Bitmap.h>
#include <IconUtils.h>
#include <Message.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Roster.h>
#include <Window.h>

#include <libinterface/BitmapMenuItem.h>
#include <libinterface/BitmapUtils.h>

#include "AccountManager.h"
#include "BitmapView.h"
#include "Caya.h"
#include "CayaMessages.h"
#include "CayaProtocolMessages.h"
#include "CayaUtils.h"
#include "NicknameTextControl.h"
#include "ReplicantStatusView.h"
#include "ReplicantMenuItem.h"

#include <stdio.h>

extern "C" _EXPORT BView *instantiate_deskbar_item(void);


// The following handler is added to the Deskbar's looper
// to receive notifications from Caya
class ReplicantHandler : public BHandler {
public:
				 ReplicantHandler(const char* name, ReplicantStatusView* target)
				 	: BHandler(name)
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
				fTarget->SetStatus((CayaStatus)status);
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
	delete fResources;
	delete fCayaMsg;
	delete fReplicantHandler;
//	delete fStatusMenu;
//	delete fReplicantMenu;

	// TODO: Use a list for that
	// maybe our List wrapper to std::list
	delete fConnectingIcon;
	delete fCayaIcon;
	delete fOfflineIcon;
	delete fBusyIcon;
	delete fAwayIcon;
}


void
ReplicantStatusView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case CAYA_REPLICANT_STATUS_SET:
		{
			int32 status;

			if (msg->FindInt32("status", &status) != B_OK)
				return;

			SetStatus((CayaStatus)status);
			fCayaMsg->SendMessage(msg);
			break;
		}
		case CAYA_REPLICANT_EXIT:
		case CAYA_SHOW_SETTINGS:
		case CAYA_REPLICANT_SHOW_WINDOW:
		case CAYA_REPLICANT_MESSENGER:
			fCayaMsg->SendMessage(msg);
			break;
		default:
			BView::MessageReceived(msg);
	}
}


void
ReplicantStatusView::SetStatus(CayaStatus status)
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
		case CAYA_AWAY:
		case CAYA_EXTENDED_AWAY:
			fIcon = fAwayIcon;
		break;
		case CAYA_DO_NOT_DISTURB:
			fIcon = fBusyIcon;
		break;
		case CAYA_OFFLINE:
			fIcon = fOfflineIcon;
		break;
		default:
			fIcon = fCayaIcon;
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
		status = archive->AddString("add_on", CAYA_SIGNATURE);

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

	BMessage msg(CAYA_REPLICANT_MESSENGER);
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

	unsigned long buttons;
	if (LockLooper()) {
		GetMouse(&point, &buttons, false);
		UnlockLooper();
	}
	if (buttons & B_PRIMARY_MOUSE_BUTTON) {
		// Show / Hide Window command
		BMessage msg(CAYA_REPLICANT_SHOW_WINDOW);
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
	fCayaMsg = new BMessenger(CAYA_SIGNATURE);

	fResources = CayaResources();

	//Get icons from resources
	fConnectingIcon = _GetIcon(kConnectingReplicant);
	fCayaIcon = _GetIcon(kCayaIconReplicant);
	fOfflineIcon = _GetIcon(kOfflineReplicant);
	fIcon = fOfflineIcon;
	fBusyIcon = _GetIcon(kBusyReplicant);
	fAwayIcon = _GetIcon(kAwayReplicant);
	fExitMenuIcon = _GetIcon(kExitMenuReplicant);

	// Build the replicant menu
	_BuildMenu();
}


BBitmap*
ReplicantStatusView::_GetIcon(const uint32 id)
{
	BBitmap* icon = IconFromResources(fResources, id, B_MINI_ICON);
	return icon;
}


void
ReplicantStatusView::_BuildMenu()
{
	// Status menu
	//fStatusMenu = new BPopUpMenu("Status", false, false);
	fReplicantMenu = new BPopUpMenu(" -  ", false, false);
	// Add status menu items
	int32 s = CAYA_ONLINE;
	while (s >= CAYA_ONLINE && s < CAYA_STATUSES) {
		if (s == CAYA_EXTENDED_AWAY) {
			s++;
			continue;
		}
		BMessage* msg = new BMessage(CAYA_REPLICANT_STATUS_SET);
		msg->AddInt32("status", s);

		ReplicantMenuItem* item = new ReplicantMenuItem(
			CayaStatusToString((CayaStatus)s), (CayaStatus)s);
		fReplicantMenu->AddItem(item);

		// Add items for custom messages
		if (s == CAYA_ONLINE/* || s == CAYA_DO_NOT_DISTURB*/) {
			item = new ReplicantMenuItem("Custom...", (CayaStatus) s, true);
			fReplicantMenu->AddItem(item);
			fReplicantMenu->AddItem(new BSeparatorItem());
		}

		// Mark offline status by default
		if (s == CAYA_OFFLINE)
			item->SetMarked(true);
		s++;
	}

	//fReplicantMenu->AddItem(fStatusMenu);
	fReplicantMenu->AddItem(new BSeparatorItem());

	fReplicantMenu->AddItem(new BitmapMenuItem("Preferences ",
		new BMessage(CAYA_SHOW_SETTINGS), fCayaIcon));

	fReplicantMenu->AddItem(new BitmapMenuItem("Exit",
		new BMessage(CAYA_REPLICANT_EXIT), fExitMenuIcon));

	//fStatusMenu->SetTargetForItems(fReplicantMenu);
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
