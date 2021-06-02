/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef JOINWINDOW_H
#define JOINWINDOW_H

#include <Window.h>

#include "Server.h"

class BMenu;
class BMenuField;
class BMessenger;
class BTextControl;


/* A window used to specify a room to join. */
class JoinWindow : public BWindow {
public:
	JoinWindow(BMessenger* messenger, AccountInstances accounts);

			void		MessageReceived(BMessage* message);

private:
			void		_InitInterface();
			BMenu*		_CreateAccountMenu();

	BMessenger*			fTarget;
	AccountInstances	fAccounts;

	BMenuField*			fMenuField;
	BTextControl*		fTextBox;

	int32				fSelectedAcc;
};


#endif // JOINWINDOW_H

