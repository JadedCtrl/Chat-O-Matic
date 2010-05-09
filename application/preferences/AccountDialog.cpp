/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Button.h>
#include <ControlLook.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <TextControl.h>
#include <String.h>

#include <libinterface/Divider.h>

#include "AccountDialog.h"
#include "AccountView.h"
#include "ProtocolSettings.h"

const uint32 kCancel       = 'CANC';
const uint32 kOK           = 'SAVE';


AccountDialog::AccountDialog(const char* title, CayaProtocol* cayap,
							 const char* account = NULL)
	: BWindow(BRect(0, 0, 1, 1), title, B_MODAL_WINDOW, B_NOT_RESIZABLE |
		B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE)
{
	fSettings = new ProtocolSettings(cayap);

	fAccountName = new BTextControl("accountName", "Account name:", NULL, NULL);
	fAccountName->SetFont(be_bold_font);
	if (account) {
		fAccountName->SetText(account);
		fAccountName->SetEnabled(false);
	} else
		fAccountName->MakeFocus(true);

	Divider* divider = new Divider("divider", B_WILL_DRAW);

	fTop = new AccountView("top");
	if (account)
		fSettings->Load(account, fTop);
	else
		fSettings->LoadTemplate(fTop);

	BButton* cancel = new BButton("Cancel", new BMessage(kCancel));
	BButton* ok = new BButton("OK", new BMessage(kOK));

	const float spacing = be_control_look->DefaultItemSpacing();

	SetLayout(new BGroupLayout(B_VERTICAL, spacing));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, spacing)
		.Add(fAccountName)
		.Add(divider)
		.Add(fTop)
		.AddGroup(B_HORIZONTAL, spacing)
			.AddGlue()
			.Add(cancel)
			.Add(ok)
		.End()
		.AddGlue()
		.SetInsets(spacing, spacing, spacing, 0)
	);

	CenterOnScreen();
}


void
AccountDialog::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case kOK:
			if (fSettings->Save(fAccountName->Text(), fTop) == B_OK)
				Close();
// TODO: Error!
			break;
		case kCancel:
			Close();
			break;
		case kChanged:
			msg->PrintToStream();
			break;
		default:
			BWindow::MessageReceived(msg);
	}
}
