/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "MatrixProtocol.h"

#include <Catalog.h>
#include <Messenger.h>
#include <Roster.h>

#include <libinterface/BitmapUtils.h>

#include <ChatProtocolMessages.h>
#include <UserStatus.h>

#include "Matrix.h"
#include "MatrixMessages.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MatrixProtocol"


status_t
connect_thread(void* data)
{
	MatrixProtocol* protocol = (MatrixProtocol*)data;
	while (true) {
		BMessage* msg = new BMessage(receive_message());
		switch (msg->what) {
			case MATRIX_ACCOUNT_REGISTERED:
				protocol->RegisterApp((team_id)msg->GetInt64("team_id", -1));
				break;
			default:
				protocol->SendMessage(msg);
		}
	}
}


BMessage
receive_message()
{
	thread_id sender;
	int32 size = receive_data(&sender, NULL, 0);
	char buffer[size];
	receive_data(&sender, buffer, size);
	BMessage temp;
	temp.Unflatten(buffer);
	return temp;
}


MatrixProtocol::MatrixProtocol()
	:
	fAppMessenger(NULL),
	fAppTeam(-1)
{
}


MatrixProtocol::~MatrixProtocol()
{
	Shutdown();
}


status_t
MatrixProtocol::Init(ChatProtocolMessengerInterface* interface)
{
	fMessenger = interface;
	return B_OK;
}


status_t
MatrixProtocol::Shutdown()
{
	_SendMatrixMessage(new BMessage(B_QUIT_REQUESTED));
	kill_thread(fRecvThread);
	return B_OK;
}


status_t
MatrixProtocol::UpdateSettings(BMessage* settings)
{
	fRecvThread = spawn_thread(connect_thread, "moon_w_blackjack_and_hookers",
		B_NORMAL_PRIORITY, (void*)this);

	if (fRecvThread < B_OK)
		return B_ERROR;

	settings->AddInt64("thread_id", fRecvThread);
	fSettings = new BMessage(*settings);
	return B_OK;
}


status_t
MatrixProtocol::Process(BMessage* msg)
{
	int32 im_what = msg->GetInt32("im_what", -1);

	switch (im_what) {
		case IM_SET_OWN_STATUS:
		{
			int32 status = msg->GetInt32("status", -1);
			switch (status) {
				case STATUS_ONLINE:
					resume_thread(fRecvThread);
					if (fAppMessenger == NULL || fAppMessenger->IsValid() == false)
						_StartApp();
					break;
				case STATUS_OFFLINE:
				{
					_SendMatrixMessage(new BMessage(B_QUIT_REQUESTED));
					kill_thread(fRecvThread);

					delete fAppMessenger;
					fAppMessenger = NULL;
					fAppTeam = -1;
					break;
				}
				default:
					_SendMatrixMessage(msg);
			}
		}
		default:
			_SendMatrixMessage(msg);
	}
	return B_OK;
}


BMessage
MatrixProtocol::SettingsTemplate(const char* name)
{
	BMessage settings;
	if (strcmp(name, "account") == 0)
		settings = _AccountTemplate();
	else if (strcmp(name, "join_room") == 0 || strcmp(name, "create_room") == 0)
		settings = _RoomTemplate();
	else if (strcmp(name, "roster") == 0)
		settings = _RosterTemplate();
	return settings;
}


BObjectList<BMessage>
MatrixProtocol::Commands()
{
	return BObjectList<BMessage>();
}


BBitmap*
MatrixProtocol::Icon() const
{
	return NULL;
//	return ReadNodeIcon(fAddOnPath.Path(), B_LARGE_ICON, true);
}


void
MatrixProtocol::SendMessage(BMessage* msg)
{
	msg->AddString("protocol", Signature());
	fMessenger->SendMessage(msg);
}


void
MatrixProtocol::RegisterApp(team_id team)
{
	if (team < 0)
		return;
	fAppTeam = team;
	fAppMessenger = new BMessenger(NULL, team);
}


void
MatrixProtocol::_SendMatrixMessage(BMessage* msg)
{
	msg->AddString("protocol", MATRIX_ADDON);
	if (fAppMessenger != NULL && fAppMessenger->IsValid())
		fAppMessenger->SendMessage(msg);
}


void
MatrixProtocol::_StartApp()
{
	BMessage* start = new BMessage(*fSettings);
	start->what = MATRIX_REGISTER_ACCOUNT;

	BRoster roster;
	if (roster.Launch(MATRIX_SIGNATURE, start) == B_OK)
		snooze(100000);
}


BMessage
MatrixProtocol::_AccountTemplate()
{
	BMessage settings;

	BMessage server;
	server.AddString("name", "server");
	server.AddString("description", B_TRANSLATE("Homeserver:"));
	server.AddString("default", "matrix.org");
	server.AddString("error", B_TRANSLATE("Please enter a valid server address."));
	server.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &server);

	BMessage user;
	user.AddString("name", "username");
	user.AddString("description", B_TRANSLATE("Username:"));
	user.AddString("error", B_TRANSLATE("You need a username in order to connect!"));
	user.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &user);

	BMessage password;
	password.AddString("name", "password"); password.AddString("description", B_TRANSLATE("Password:"));
	password.AddString("error", B_TRANSLATE("Without a password, how will love survive?"));
	password.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &password);

	BMessage session;
	session.AddString("name", "session");
	session.AddString("description", B_TRANSLATE("Session name:"));
	session.AddInt32("type", B_STRING_TYPE);
	session.AddString("default", "Chat-O-Matic [Haiku]");
	settings.AddMessage("setting", &session);

	return settings;
}


BMessage
MatrixProtocol::_RoomTemplate()
{
	BMessage settings;

	BMessage id;
	id.AddString("name", "chat_id");
	id.AddString("description", B_TRANSLATE("Channel:"));
	id.AddString("error", B_TRANSLATE("Please enter a channel― skipping it doesn't make sense!"));
	id.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &id);

	return settings;
}


BMessage
MatrixProtocol::_RosterTemplate()
{
	BMessage settings;

	BMessage nick;
	nick.AddString("name", "user_id");
	nick.AddString("description", B_TRANSLATE("User ID:"));
	nick.AddString("error", B_TRANSLATE("How can someone be your friend if you don't know their ID?"));
	nick.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &nick);

	return settings;
}
