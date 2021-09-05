/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "MatrixApp.h"

#include <iostream>
#include <string>

#include <Catalog.h>
#include <MessageRunner.h>
#include <Roster.h>

#include <ChatOMatic.h>
#include <ChatProtocolMessages.h>
#include <Flags.h>
#include <UserStatus.h>

#include "Matrix.h"
#include "MatrixMessages.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MatrixApp"


std::shared_ptr<mtx::http::Client> client = nullptr;
MatrixApp* m_app = NULL;


int
main(int arc, char** argv)
{
	MatrixApp app;
	app.Run();
	return 0;
}


MatrixApp::MatrixApp()
	:
	BApplication(MATRIX_SIGNATURE),
	fUser(NULL),
	fPassword(NULL),
	fInitStatus(B_NOT_INITIALIZED),
	fProtoThread(-1)
{
	new BMessageRunner(this, new BMessage(CHECK_APP), 10000000, -1);
}


void
MatrixApp::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case MATRIX_REGISTER_ACCOUNT:
		{
			int64 thread_id;
			if (msg->FindInt64("thread_id", &thread_id) != B_OK)
				break;

			fProtoThread = thread_id;
			fUser = msg->FindString("username");
			fSession = msg->GetString("session", "Chat-O-Matic [Haiku]");
			fServer = msg->FindString("server");
			fPassword = msg->FindString("password");

			app_info info;
			GetAppInfo(&info);
			BMessage registerApp(MATRIX_ACCOUNT_REGISTERED);
			registerApp.AddInt64("team_id", info.team);
			SendMessage(registerApp);

			Connect();
			break;
		}
		case CHECK_APP:
		{
			BRoster roster;
			if (roster.IsRunning(APP_SIGNATURE) == false)
				Quit();
			break;
		}
		default:
			BApplication::MessageReceived(msg);
	}
}


void
MatrixApp::ImMessage(BMessage* msg)
{
	int32 im_what = msg->GetInt32("im_what", -1);
	switch (im_what) {
		case IM_SET_OWN_STATUS:
		{
		}
		default: {
			std::cout << "Unhandled message for Matrix:\n";
			msg->PrintToStream();
		}
	}
}


void
MatrixApp::Connect()
{
	client = std::make_shared<mtx::http::Client>(fServer.String());
	client->set_device_id(fSession.String());
	client->login(fUser.String(), fPassword.String(),
		[this](const mtx::responses::Login &res, mtx::http::RequestErr err)
		{
			if (err) {
				SendError("Error occured during login, please try again.", err,
					true);
				fInitStatus = B_ERROR;
			}
			else {
				client->set_access_token(res.access_token);
				fInitStatus = B_OK;
				StartLoop();
			}
		});
}


void
MatrixApp::StartLoop()
{
	client->get_profile(client->user_id().to_string(),
		[this](const mtx::responses::Profile &res, mtx::http::RequestErr err)
		{
			if (err) {
				print_error(err, "Failed getting own info after login…");
				StartLoop();
				snooze(1000000);
				return;
			}
			BMessage ready(IM_MESSAGE);
			ready.AddInt32("im_what", IM_PROTOCOL_READY);
			SendMessage(ready);

			BMessage init(IM_MESSAGE);
			init.AddInt32("im_what", IM_OWN_CONTACT_INFO);
			init.AddString("user_id", client->user_id().to_string().c_str());
			init.AddString("user_name", res.display_name.c_str());
			SendMessage(init);
		});

	BMessage status(IM_MESSAGE);
	status.AddInt32("im_what", IM_OWN_STATUS_SET);
	status.AddInt32("status", (int32)STATUS_ONLINE);
	SendMessage(status);

	mtx::http::SyncOpts opts;
	opts.timeout = 0;
	client->sync(opts, &initial_sync_handler);
	client->close();
}


void
MatrixApp::SendMessage(BMessage msg)
{
	if (fProtoThread <= 0)
		return;

	ssize_t size = msg.FlattenedSize();
	char buffer[size];

	send_data(fProtoThread, size, NULL, 0);
	msg.Flatten(buffer, size);
	send_data(fProtoThread, 0, buffer, size);
}


void
MatrixApp::SendError(const char* message, mtx::http::RequestErr err,
	bool fatal)
{
	print_error(err);

	BString detail;
	if (!err->matrix_error.error.empty())
		detail << err->matrix_error.error.c_str();

	BMessage error(IM_ERROR);
	error.AddString("error", message);
	error.AddString("detail", detail);
	SendMessage(error);

	if (fatal == true) {
		BMessage disable(IM_MESSAGE);
		disable.AddInt32("im_what", IM_PROTOCOL_DISABLE);
		SendMessage(disable);
	}
}


void
print_error(mtx::http::RequestErr err, const char* message)
{
	if (message != NULL)
		std::cerr << message << " ― ";
	std::cerr << err->status_code << " : " << err->error_code;
	if (!err->matrix_error.error.empty())
		std::cerr << " : " << err->matrix_error.error;
	std::cerr << std::endl;
}


void
initial_sync_handler(const mtx::responses::Sync &res, mtx::http::RequestErr err)
{
	mtx::http::SyncOpts opts;

	if (err) {
		print_error(err, "Error occured during initial sync. Retrying…");
		if ((int)err->status_code != 200) {
			opts.timeout = 0;
			client->sync(opts, &initial_sync_handler);
		}
		return;
	}

	room_sync(res.rooms);

	opts.since = res.next_batch;
	client->set_next_batch_token(res.next_batch);
	client->sync(opts, &sync_handler);
}


// Callback to executed after a /sync request completes.
void
sync_handler(const mtx::responses::Sync &res, mtx::http::RequestErr err)
{
	mtx::http::SyncOpts opts;

	if (err) {
		print_error(err, "Error occured during sync. Retrying…");
		opts.since = client->next_batch_token();
		client->sync(opts, &sync_handler);
		return;
	}

	room_sync(res.rooms);

	opts.since = res.next_batch;
	client->set_next_batch_token(res.next_batch);
	client->sync(opts, &sync_handler);
}


void
room_sync(mtx::responses::Rooms rooms)
{
	MatrixApp* app = (MatrixApp*)be_app;

	std::map<std::string, mtx::responses::JoinedRoom> joined = rooms.join;
	for (std::map<std::string, mtx::responses::JoinedRoom>::iterator iter
				= joined.begin();
			iter != joined.end();
			++iter)
	{
		const char* chat_id = iter->first.c_str();
		mtx::responses::JoinedRoom room = iter->second;

		if (app->fRoomList.HasString(BString(chat_id)) == false) {
			BMessage joinedMsg(IM_MESSAGE);
			joinedMsg.AddInt32("im_what", IM_ROOM_JOINED);
			joinedMsg.AddString("chat_id", chat_id);
			((MatrixApp*)be_app)->SendMessage(joinedMsg);

			app->fRoomList.Add(BString(chat_id));
		}

		for (mtx::events::collections::TimelineEvents &ev : room.timeline.events)
			if (auto event = std::get_if<mtx::events::RoomEvent<mtx::events::msg::Text>>(&ev);
					event != nullptr)
			{
				BMessage msg(IM_MESSAGE);
				msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
				msg.AddString("body", event->content.body.c_str());
				msg.AddString("chat_id", chat_id);
				msg.AddString("user_id", event->sender.c_str());
				app->SendMessage(msg);
			}
	}

	std::map<std::string, mtx::responses::LeftRoom> left = rooms.leave;
	for (std::map<std::string, mtx::responses::LeftRoom>::iterator iter
				= left.begin();
			iter != left.end();
			++iter)
	{
		const char* chat_id = iter->first.c_str();
		if (app->fRoomList.HasString(BString(chat_id)) == true)
			app->fRoomList.Remove(BString(chat_id));

		BMessage leftMsg(IM_MESSAGE);
		leftMsg.AddInt32("im_what", IM_ROOM_LEFT);
		leftMsg.AddString("chat_id", ((std::string)iter->first).c_str());
		((MatrixApp*)be_app)->SendMessage(leftMsg);
	}
}
