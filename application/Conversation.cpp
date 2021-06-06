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
#include "CayaRenderView.h"
#include "CayaUtils.h"
#include "ConversationItem.h"
#include "ConversationView.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "ProtocolLooper.h"
#include "ProtocolManager.h"
#include "Server.h"
#include "TheApp.h"


Conversation::Conversation(BString id, BMessenger msgn)
	:
	fID(id),
	fName(id),
	fMessenger(msgn),
	fChatView(NULL),
	fLooper(NULL),
	fIcon(NULL),
	fDateFormatter()
{
	fConversationItem = new ConversationItem(fName.String(), this);
	RegisterObserver(fConversationItem);
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
			GetView()->MessageReceived(msg);
			break;
		}
		case IM_MESSAGE_SENT:
		{
			_LogChatMessage(msg);
			GetView()->MessageReceived(msg);
			break;
		}
		case IM_SEND_MESSAGE:
		{
			fMessenger.SendMessage(msg);
			break;
		}
		case IM_LOGS_RECEIVED:
		default:
			GetView()->MessageReceived(msg);
	}
}


void
Conversation::ObserveString(int32 what, BString str)
{
	GetView()->InvalidateUserList();
}


void
Conversation::ObserveInteger(int32 what, int32 value)
{
	GetView()->InvalidateUserList();
}


void
Conversation::ObservePointer(int32 what, void* ptr)
{
	GetView()->InvalidateUserList();
}


void
Conversation::SetNotifySubject(const char* subject)
{
	if (BString(subject) == fSubject)
		return;

	fSubject = subject;
	NotifyString(STR_ROOM_SUBJECT, fSubject.String());
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


BBitmap*
Conversation::ProtocolBitmap() const
{
	CayaProtocol* protocol = fLooper->Protocol();
	CayaProtocolAddOn* addOn
		= ProtocolManager::Get()->ProtocolAddOn(protocol->Signature());

	return addOn->ProtoIcon();
}


BBitmap*
Conversation::IconBitmap() const
{
	return fIcon;
}


BString
Conversation::GetName() const
{
	return fName;
}


ConversationView*
Conversation::GetView()
{
	if (fChatView != NULL)
		return fChatView;

	fChatView = new ConversationView(this);

	if (fLooper->Protocol()->SaveLogs() == false)
		return fChatView;

	BMessage logMsg;
	if (_GetChatLogs(&logMsg) == B_OK)
		fChatView->MessageReceived(&logMsg);

	RegisterObserver(fChatView);
	return fChatView;
}


void
Conversation::ShowView(bool typing, bool userAction)
{
	((TheApp*)be_app)->GetMainWindow()->SetConversation(this);
}


ConversationItem*
Conversation::GetListItem()
{
	return fConversationItem;
}


UserMap
Conversation::Users()
{
	return fUsers;
}


User*
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
	msg.AddString("user_name", user->GetName());
	_EnsureUser(&msg);
}


void
Conversation::RemoveUser(User* user)
{
	fUsers.RemoveItemFor(user->GetId());
	user->UnregisterObserver(this);
	GetView()->UpdateUserList(fUsers);
}


BString
Conversation::OwnUserId()
{
	return _GetServer()->GetOwnContact();
}


void
Conversation::SetRole(BString id, Role* role)
{
	Role* oldRole = fRoles.ValueFor(id);
	if (oldRole != NULL) {
		fRoles.RemoveItemFor(id);
		delete oldRole;
	}

	fRoles.AddItem(id, role);
}


Role*
Conversation::GetRole(BString id)
{
	return fRoles.ValueFor(id);
}


void
Conversation::_LogChatMessage(BMessage* msg)
{
	BString date;
	fDateFormatter.Format(date, time(0), B_SHORT_DATE_FORMAT, B_MEDIUM_TIME_FORMAT);

	BString id = msg->FindString("user_id");
	BString body = msg->FindString("body");

	// Binary logs
	// TODO: Don't hardcode 21, expose maximum as a setting
	BStringList users, bodies;

	BMessage logMsg;
	if (_GetChatLogs(&logMsg) == B_OK) {
		logMsg.FindStrings("body", &bodies);
		logMsg.FindStrings("user_id", &users);
		bodies.Remove(21);
		users.Remove(21);
		bodies.Add(body, 0);
		users.Add(id, 0);
	}

	BMessage newLogMsg(IM_MESSAGE);
	newLogMsg.AddInt32("im_what", IM_LOGS_RECEIVED);
	newLogMsg.AddStrings("body", bodies);
	newLogMsg.AddStrings("user_id", users);

	BFile logFile(fLogPath.Path(), B_READ_WRITE | B_OPEN_AT_END | B_CREATE_FILE);
	WriteAttributeMessage(&logFile, "logs", &newLogMsg);

	// Plain-text logs
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
	logLine << body;
	logLine << "\n";

	logFile.Write(logLine.String(), logLine.Length());
}


status_t
Conversation::_GetChatLogs(BMessage* msg)
{
	_EnsureLogPath();

	BFile logFile(fLogPath.Path(), B_READ_WRITE | B_CREATE_FILE);

	return ReadAttributeMessage(&logFile, "logs", msg);
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


User*
Conversation::_EnsureUser(BMessage* msg)
{
	BString id = msg->FindString("user_id");
	if (id.IsEmpty() == true) return NULL;

	User* user = UserById(id);
	User* serverUser = _GetServer()->UserById(id);

	if (user == NULL && serverUser != NULL) {
		fUsers.AddItem(id, serverUser);
		user = serverUser;
		GetView()->UpdateUserList(fUsers);
	}
	else if (user == NULL) {
		user = new User(id, _GetServer()->Looper());
		user->SetProtocolLooper(fLooper);

		_GetServer()->AddUser(user);
		fUsers.AddItem(id, user);
		GetView()->UpdateUserList(fUsers);
	}
	user->RegisterObserver(this);
	return user;
}


Server*
Conversation::_GetServer()
{
	return ((TheApp*)be_app)->GetMainWindow()->GetServer();
}


