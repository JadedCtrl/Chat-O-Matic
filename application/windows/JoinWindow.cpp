/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "JoinWindow.h"

#include <Alert.h>
#include <Button.h>
#include <LayoutBuilder.h>
#include <Messenger.h>
#include <StringView.h>

#include "CayaProtocolMessages.h"


const uint32 kJoinRoom = 'JWjr';
const uint32 kAccSelected = 'JWas';


JoinWindow::JoinWindow(BMessenger* messenger, AccountInstances accounts)
	:
	BWindow(BRect(0, 0, 400, 100), "Join a room", B_FLOATING_WINDOW, 0),
	fTarget(messenger),
	fAccounts(accounts),
	fSelectedAcc(0)
{
	_InitInterface();

	CenterOnScreen();
}


void
JoinWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case kAccSelected:
		{
			int32 index;
			if (msg->FindInt32("index", &index) == B_OK)
				fSelectedAcc = index;
			break;
		}

		case kJoinRoom:
		{
			BString roomId = fTextBox->Text();
			BString selected = fMenuField->Menu()->ItemAt(fSelectedAcc)->Label();
			int64 instanceId = fAccounts.ValueFor(selected);
			
			if (roomId.IsEmpty() == true) {
				BAlert* alert = new BAlert("No room ID", "You can't join a room "
					"with no name― you need to specify a room ID.", "OK", "", "",
					B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_IDEA_ALERT);
				alert->Go();
				return;
			}

			BMessage* joinMsg = new BMessage(IM_MESSAGE);
			joinMsg->AddInt32("im_what", IM_JOIN_ROOM);
			joinMsg->AddInt64("instance", instanceId);
			joinMsg->AddString("chat_id", roomId);
			fTarget->SendMessage(joinMsg);

			Quit();
			break;
		}

		default:
			BWindow::MessageReceived(msg);
	}
}


void
JoinWindow::_InitInterface()
{
	fMenuField = new BMenuField("accountMenuField", NULL, _CreateAccountMenu());
	BButton* join = new BButton("Join", new BMessage(kJoinRoom));
	fTextBox = new BTextControl("Room ID:", "", NULL);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.Add(fTextBox)
		.AddGroup(B_HORIZONTAL)
			.Add(fMenuField)
			.AddGlue()
			.Add(new BButton("Cancel", new BMessage(B_QUIT_REQUESTED)))
			.Add(join)
		.End()
	.End();

	fTextBox->MakeFocus(true);
	join->MakeDefault(true);
}


BMenu*
JoinWindow::_CreateAccountMenu()
{
	BMenu* menu = new BMenu("accountMenu");

	for (int i = 0; i < fAccounts.CountItems(); i++)
		menu->AddItem(new BMenuItem(fAccounts.KeyAt(i).String(),
									new BMessage(kAccSelected)));

	menu->SetRadioMode(true);
	menu->SetLabelFromMarked(true);
	menu->ItemAt(fSelectedAcc)->SetMarked(true);

	if (fAccounts.CountItems() == 0)
		menu->SetEnabled(false);

	return menu;
}


