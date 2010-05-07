/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "AccountManager.h"
#include "NotifyMessage.h"
#include "Server.h"
#include "TheApp.h"

static AccountManager* fInstance = NULL;


AccountManager::AccountManager()
	: fStatus(CAYA_OFFLINE)
{
	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	RegisterObserver(theApp->GetMainWindow());
}


AccountManager::~AccountManager()
{
	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	UnregisterObserver(theApp->GetMainWindow());
}


AccountManager*
AccountManager::Get()
{
	if (fInstance == NULL)
		fInstance = new AccountManager();
	return fInstance;
}


void
AccountManager::SetNickname(BString nick)
{
	// Create message
	BMessage* msg = new BMessage(IM_MESSAGE);
	msg->AddInt32("im_what", IM_SET_NICKNAME);
	msg->AddString("nick", nick);

	// Send message
	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	MainWindow* win = theApp->GetMainWindow();
	win->GetServer()->SendProtocolMessage(msg);
}


CayaStatus
AccountManager::Status() const
{
	return fStatus;
}


void
AccountManager::SetStatus(CayaStatus status, const char* str)
{
	if (fStatus != status) {
		// Create status change message
		BMessage* msg = new BMessage(IM_MESSAGE);
		msg->AddInt32("im_what", IM_SET_STATUS);
		msg->AddInt32("status", (int32)status);
		if (str != NULL)
			msg->AddString("message", str);

		// Send message
		TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
		MainWindow* win = theApp->GetMainWindow();
		win->GetServer()->SendProtocolMessage(msg);

		// Notify status change
		fStatus = status;
		NotifyInteger(INT_ACCOUNT_STATUS, (int32)fStatus);
	}
}
