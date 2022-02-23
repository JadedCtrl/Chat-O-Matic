/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "TemplateWindow.h"

#include <Alert.h>
#include <Button.h>
#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>
#include <TextControl.h>
#include <String.h>

#include "AccountsMenu.h"
#include "ChatProtocolMessages.h"
#include "Server.h"
#include "TemplateView.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TemplateWindow"


const uint32 kOK		= 'save';
const uint32 kAccSelected = 'JWas';



TemplateWindow::TemplateWindow(const char* title, const char* templateType,
	BMessage* msg, bigtime_t instance)
	:
	BWindow(BRect(0, 0, 400, 100), title, B_FLOATING_WINDOW,
		B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fSelectedAcc(instance),
	fTemplate(NULL),
	fTemplateType(templateType),
	fMessage(msg),
	fTarget(NULL)
{
	_InitInterface(instance);
	_LoadTemplate();
	CenterOnScreen();
}


TemplateWindow::TemplateWindow(const char* title, ProtocolTemplate* temp,
	BMessage* msg, bigtime_t instance)
	:
	BWindow(BRect(0, 0, 400, 100), title, B_FLOATING_WINDOW,
		B_NOT_RESIZABLE | B_AUTO_UPDATE_SIZE_LIMITS | B_CLOSE_ON_ESCAPE),
	fSelectedAcc(-1),
	fTemplate(temp),
	fMessage(msg)
{
	_InitInterface(instance);
	CenterOnScreen();

	fTemplate->Load(fTemplateView);
	fTemplateView->AttachedToWindow();
	fTemplateView->MakeFocus(true);
}


void
TemplateWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case IM_MESSAGE: {
			// If IM_MESSAGE, assume it should be treated as current settings
			if (fTemplate == NULL)
				break;
			for (int i = 0; fTemplateView->CountChildren(); i++)
				fTemplateView->RemoveChild(fTemplateView->ChildAt(i));
			fTemplate->Load(fTemplateView, msg);
			break;
		}
		case kOK: {
			// Save account settings
			if (fTemplate == NULL || fTemplateView == NULL)
				break;
			BString error = B_TRANSLATE("Some items are empty. Please make "
				"sure to fill out every item.");
			BMessage* settings = new BMessage(*fMessage);
			status_t result = fTemplate->Save(fTemplateView, settings, &error);

			if (result != B_OK) {
				BAlert* alert = new BAlert("", error.String(),
					B_TRANSLATE("OK"), NULL, NULL, B_WIDTH_AS_USUAL,
					B_WARNING_ALERT);
				alert->Go();
				break;
			}

			ProtocolLooper* looper = Server::Get()->GetProtocolLooper(fSelectedAcc);
			if (looper == NULL)
				break;
			looper->PostMessage(settings);
			Close();
			break;
		}
		case kAccSelected:
		{
			int64 instance;
			if (msg->FindInt64("instance", &instance) == B_OK)
				fSelectedAcc = instance;
			_LoadTemplate();
			break;
		}
		default:
			BWindow::MessageReceived(msg);
	}
}


void
TemplateWindow::SetTarget(BHandler* target)
{
	fTarget = target;
}


void
TemplateWindow::_InitInterface(bigtime_t instance)
{
	fTemplateView = new TemplateView("template");
	AccountInstances accounts = Server::Get()->GetActiveAccounts();

	if (instance > -1) {
		BMenu* accountMenu = new BMenu("accountMenu");
		BString name = "N/A";

		for (int i = 0; i < accounts.CountItems(); i++)
			if (accounts.ValueAt(i) == instance) {
				name = accounts.KeyAt(i);
				break;
			}
		accountMenu->AddItem(new BMenuItem(name.String(), NULL));
		accountMenu->SetLabelFromMarked(true);
		accountMenu->ItemAt(0)->SetMarked(true);
		accountMenu->SetEnabled(false);

		fMenuField = new BMenuField("accountMenuField", NULL, accountMenu);
	}
	else {
		AccountsMenu* accountMenu = new AccountsMenu("accountMenu",
			BMessage(kAccSelected));
		fMenuField = new BMenuField("accountMenuField", NULL, accountMenu);

		fSelectedAcc = accountMenu->GetDefaultSelection();
	}

	BButton* fOkButton = new BButton(B_TRANSLATE("OK"), new BMessage(kOK));
	if (accounts.CountItems() <= 0)
		fOkButton->SetEnabled(false);
	fOkButton->MakeDefault(true);

	const float spacing = be_control_look->DefaultItemSpacing();


	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fTemplateView)
		.AddGroup(B_HORIZONTAL)
			.Add(fMenuField)
			.AddGlue()
			.Add(new BButton(B_TRANSLATE("Cancel"),
				new BMessage(B_QUIT_REQUESTED)))
			.Add(fOkButton)
		.End()
	.End();
}


void
TemplateWindow::_LoadTemplate()
{
	if (fTemplateType.IsEmpty() == true)
		return;

	ProtocolLooper* looper = Server::Get()->GetProtocolLooper(fSelectedAcc);
	if (looper == NULL)
		return;

	fTemplate = new ProtocolTemplate(looper->Protocol(), fTemplateType.String());

	for (int i = 0; fTemplateView->CountChildren(); i++)
		fTemplateView->RemoveChild(fTemplateView->ChildAt(i));

	fTemplate->Load(fTemplateView);
	fTemplateView->AttachedToWindow();
	fTemplateView->MakeFocus(true);
}
