/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_DIALOG_H
#define _ACCOUNT_DIALOG_H

#include <Window.h>

class BTextControl;

class AccountView;
class CayaProtocol;
class ProtocolSettings;

class AccountDialog : public BWindow {
public:
						AccountDialog(const char* title, CayaProtocol* cayap,
					    	          const char* account = NULL);

	virtual	void		MessageReceived(BMessage* msg);

private:
	ProtocolSettings*	fSettings;
	AccountView*		fTop;
	BTextControl*		fAccountName;
};

#endif	// _ACCOUNT_DIALOG_H
