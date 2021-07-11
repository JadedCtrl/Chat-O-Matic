/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "PurpleDialog.h"

#include <libpurple/purple.h>

#include <Button.h>
#include <LayoutBuilder.h>
#include <StringView.h>
#include <TextView.h>


PurpleDialog::PurpleDialog(const char* title, const char* primary,
	const char* secondary, PurpleAccount* account, va_list actions,
	size_t action_count, void* user_data)
	:
	BWindow(BRect(BPoint(-1000, -1000), BSize(300, 250)),title,
		B_FLOATING_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL,
		B_AUTO_UPDATE_SIZE_LIMITS),
	fUserData(user_data)
{
	CenterOnScreen();
	_ParseActions(actions, action_count, PURPLE_REQUEST_ACTION);
	_InitActionInterface(primary, secondary);
}


void
PurpleDialog::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case ACTION_BUTTON:
		{
			int32 id;
			if (msg->FindInt32("index", &id) != B_OK) break;

			PurpleRequestActionCb cb = fActions.ItemAt(0)->callback.action;
			cb(fUserData, fActions.ItemAt(0)->index);
			Quit();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
PurpleDialog::_InitActionInterface(const char* label, const char* desc)
{
	BStringView* primaryLabel = new BStringView("primaryText", label);
	primaryLabel->SetExplicitAlignment(
		BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	
	BTextView* secondaryLabel = new BTextView("secondaryText");
	secondaryLabel->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	secondaryLabel->MakeEditable(false);
	secondaryLabel->SetWordWrap(true);
	secondaryLabel->SetText(desc);

	// Init buttons view
	BView* buttonsView = new BView("actionButtons", 0);
	BLayoutBuilder::Group<>(buttonsView, B_HORIZONTAL);
	for (int i = 0; i < fActions.CountItems(); i++) {
		RequestAction* action = fActions.ItemAt(i);
		BMessage* msg = new BMessage(ACTION_BUTTON);
		msg->AddInt32("index", action->index);

		BButton* button = new BButton(action->name.String(), msg);
		buttonsView->AddChild(button);
	}

	// Main layout 
	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(primaryLabel)
		.Add(secondaryLabel)
		.Add(buttonsView)
		.AddGlue()
	.End();
}


void
PurpleDialog::_ParseActions(va_list actions, int32 count,
	PurpleRequestType type)
{
	for (int i = 0; i < count; i++) {
		RequestAction* action = new RequestAction;
		action->name = va_arg(actions, const char*);
		action->index = i;
		action->type = type;

		switch (type) {
			case PURPLE_REQUEST_ACTION:
				action->callback.action = va_arg(actions, PurpleRequestActionCb);
				break;
		}
		fActions.AddItem(action);
	}

}
