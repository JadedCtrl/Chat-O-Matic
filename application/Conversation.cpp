/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Conversation.h"

#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "ChatWindow.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"
#include "WindowsManager.h"


Conversation::Conversation(BString id, BMessenger msgn)
	:
	fID(id),
	fName(id),
	fMessenger(msgn),
	fChatWindow(NULL),
	fNewWindow(true),
	fLooper(NULL)
{
}


BString
Conversation::GetId() const
{
	return fID;
}


void
Conversation::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");

	switch(im_what)
	{
		case IM_MESSAGE_RECEIVED:
		{
			_EnsureUser(msg);

			ChatWindow* win = GetChatWindow();
			ShowWindow();
			win->PostMessage(msg);
			break;
		}
		default:
			GetChatWindow()->PostMessage(msg);
	}
}


void
Conversation::ObserveString(int32 what, BString str)
{
	if (fChatWindow != NULL)
		fChatWindow->ObserveString(what, str);
}


void
Conversation::ObservePointer(int32 what, void* ptr)
{
	if (fChatWindow != NULL)
		fChatWindow->ObservePointer(what, ptr);
}


void
Conversation::ObserveInteger(int32 what, int32 val)
{
	if (fChatWindow != NULL)
		fChatWindow->ObserveInteger(what, val);
}


ChatWindow*
Conversation::GetChatWindow()
{
	if (fChatWindow == NULL)
		_CreateChatWindow();
	return fChatWindow;
}


void
Conversation::DeleteWindow()
{
	if (fChatWindow != NULL) {
		if (fChatWindow->Lock()) {
			fChatWindow->Quit();
			fChatWindow = NULL;
			fNewWindow = true;
		}
	}
}


void
Conversation::ShowWindow(bool typing, bool userAction)
{
	if (fChatWindow == NULL)
		_CreateChatWindow();

	fChatWindow->AvoidFocus(true);

	if (CayaPreferences::Item()->MoveToCurrentWorkspace)
		fChatWindow->SetWorkspaces(B_CURRENT_WORKSPACE);

	if (fNewWindow || userAction) {
		fChatWindow->AvoidFocus(false);
		fChatWindow->ShowWindow();
		fNewWindow = false;
	} else {
		if (typing) {
			if (CayaPreferences::Item()->RaiseUserIsTyping)
				fChatWindow->ShowWindow();
		} else {
			if (CayaPreferences::Item()->RaiseOnMessageReceived
			|| fChatWindow->IsHidden())
				fChatWindow->ShowWindow();
		}
	}
	fChatWindow->AvoidFocus(false);
}


void
Conversation::HideWindow()
{
	if ((fChatWindow != NULL) && !fChatWindow->IsHidden())
		fChatWindow->Hide();
}


BMessenger
Conversation::Messenger() const
{
	return fMessenger;
}


void
Conversation::SetMessenger(BMessenger messenger)
{
	fMessenger = messenger;
}


ProtocolLooper*
Conversation::GetProtocolLooper() const
{
	return fLooper;
}


void
Conversation::SetProtocolLooper(ProtocolLooper* looper)
{
	fLooper = looper;
}


BString
Conversation::GetName() const
{
	return fName;
}


UserMap
Conversation::Users()
{
	return fUsers;
}


Contact*
Conversation::UserById(BString id)
{
	bool found = false;
	return fUsers.ValueFor(id, &found);
}


void
Conversation::AddUser(User* user)
{
	BMessage msg;
	msg.AddString("user_id", user->GetId());
	_EnsureUser(&msg);
}


void
Conversation::_CreateChatWindow()
{
	fChatWindow = new ChatWindow(this);
	WindowsManager::Get()->RelocateWindow(fChatWindow);
}


Contact*
Conversation::_EnsureUser(BMessage* msg)
{
	BString id = msg->FindString("user_id");
	if (id.IsEmpty() == true) return NULL;

	Contact* user = UserById(id);
	Contact* serverUser = _GetServer()->ContactById(id);

	if (user == NULL && serverUser != NULL) {
		fUsers.AddItem(id, serverUser);
		user = serverUser;
	}
	else if (user == NULL) {
		user = new Contact(id, _GetServer()->Looper());
		user->SetProtocolLooper(fLooper);

		_GetServer()->AddContact(user);
		fUsers.AddItem(id, user);
	}

	user->RegisterObserver(this);
	return user;
}


Server*
Conversation::_GetServer()
{
	return ((TheApp*)be_app)->GetMainWindow()->GetServer();
}


