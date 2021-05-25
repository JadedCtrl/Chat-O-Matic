/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "Conversation.h"

#include <DateTimeFormat.h>
#include <Locale.h>
#include <StringList.h>

#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "CayaUtils.h"
#include "ChatWindow.h"
#include "MainWindow.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
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
	fLooper(NULL),
	fDateFormatter()
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

			_LogChatMessage(msg);
			ChatWindow* win = GetChatWindow();
			ShowWindow();
			win->PostMessage(msg);
			break;
		}
		case IM_SEND_MESSAGE:
		{
			_LogChatMessage(msg);
			fMessenger.SendMessage(msg);
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

	if (fLooper->Protocol()->SaveLogs() == false)
		return;

	BStringList logs = _GetChatLogs();
	BMessage logMsg(IM_MESSAGE);
	logMsg.AddInt32("im_what", IM_LOGS_RECEIVED);
	logMsg.AddStrings("log", logs);
	fChatWindow->ImMessage(&logMsg);
}


void
Conversation::_LogChatMessage(BMessage* msg)
{
	BString date;
	fDateFormatter.Format(date, time(0), B_SHORT_DATE_FORMAT, B_MEDIUM_TIME_FORMAT);

	BString id = msg->FindString("user_id");
	BString uname;

	if (id.IsEmpty() == false)
		uname = UserById(id)->GetName();
	else
		uname = "You";

	BString logLine("[");
	logLine << date;
	logLine << "] ";
	logLine << uname;
	logLine << ": ";
	logLine << msg->FindString("body");
	logLine << "\n";


	// TODO: Don't hardcode 21, expose maximum as a setting
	BStringList logs = _GetChatLogs();
	logs.Remove(21);
	logs.Add(logLine, 0);

	BMessage newLogMsg;
	newLogMsg.AddStrings("log", logs);

	BFile logFile(fLogPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	newLogMsg.Flatten(&logFile);
}


BStringList
Conversation::_GetChatLogs()
{
	_EnsureLogPath();

	BFile logFile(fLogPath.Path(), B_READ_WRITE | B_CREATE_FILE);
	BMessage logMsg;
	BStringList logs;

	if (logMsg.Unflatten(&logFile) == B_OK) {
		logMsg.FindStrings("log", &logs);
	}

	return logs;
}


void
Conversation::_EnsureLogPath()
{
	if (fLogPath.InitCheck() == B_OK)
		return;

	const char* sig = fLooper->Protocol()->Signature();
	CayaProtocolAddOn* protoAdd = ProtocolManager::Get()->ProtocolAddOn(sig);

	fLogPath.SetTo(CayaLogPath(protoAdd->Signature(), protoAdd->ProtoSignature()));
	fLogPath.Append(fID);
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


