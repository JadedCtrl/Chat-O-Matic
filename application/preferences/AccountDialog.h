/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_DIALOG_H
#define _ACCOUNT_DIALOG_H

#include <String.h>
#include <Window.h>

class BTextControl;

class AccountView;
class ProtocolSettings;

const uint32 kAccountSaved = 'acsd';
const uint32 kAccountRenamed = 'acrd';

class AccountDialog : public BWindow {
public:
						AccountDialog(const char* title, ProtocolSettings* settings,
									  const char* account = NULL);

			void		SetTarget(BHandler* target);

	virtual	void		MessageReceived(BMessage* msg);

private:
	ProtocolSettings*	fSettings;
	BString				fAccount;
	AccountView*		fTop;
	BTextControl*		fAccountName;
	BHandler*			fTarget;
};

#endif	// _ACCOUNT_DIALOG_H
