/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "AccountManager.h"
#include "ChatProtocolMessages.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "Server.h"
#include "TheApp.h"

#include <stdio.h>

static AccountManager* fInstance = NULL;


AccountManager::AccountManager()
	:
	fStatus(STATUS_OFFLINE),
	fReplicantMessenger(NULL)
{
	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	RegisterObserver(theApp->GetMainWindow());
}


AccountManager::~AccountManager()
{
	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	UnregisterObserver(theApp->GetMainWindow());
	delete fReplicantMessenger;
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
	msg->AddInt32("im_what", IM_SET_OWN_NICKNAME);
	msg->AddString("user_name", nick);

	// Send message
	TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
	MainWindow* win = theApp->GetMainWindow();
	win->GetServer()->SendAllProtocolMessage(msg);
}


void
AccountManager::SetReplicantMessenger(BMessenger* messenger)
{
	fReplicantMessenger = messenger;
}


UserStatus
AccountManager::Status() const
{
	return fStatus;
}


void
AccountManager::SetStatus(UserStatus status, const char* str)
{
	if (fStatus != status) {
		// Create status change message
		BMessage* msg = new BMessage(IM_MESSAGE);
		msg->AddInt32("im_what", IM_SET_OWN_STATUS);
		msg->AddInt32("status", (int32)status);
		if (str != NULL)
			msg->AddString("message", str);

		// Send message
		TheApp* theApp = reinterpret_cast<TheApp*>(be_app);
		MainWindow* win = theApp->GetMainWindow();
		win->GetServer()->SendAllProtocolMessage(msg);

		// Notify status change
		fStatus = status;
		NotifyInteger(INT_ACCOUNT_STATUS, (int32)fStatus);
		ReplicantStatusNotify((UserStatus)status);
	}
}


void
AccountManager::ReplicantStatusNotify(UserStatus status, bool wait)
{
	if(fReplicantMessenger != NULL && fReplicantMessenger->IsValid()) {
		printf("notification sent\n");
		BMessage mess(IM_OWN_STATUS_SET);
		mess.AddInt32("status", status);
		fReplicantMessenger->SendMessage(&mess);
	}
}
