/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _MATRIX_APP_H
#define _MATRIX_APP_H

#include <Application.h>
#include <String.h>
#include <StringList.h>

#include <mtx.hpp>
#include <mtxclient/http/client.hpp>
#include <mtxclient/http/errors.hpp>


const uint32 CHECK_APP = 'Paca';


class MatrixApp : public BApplication {
public:
						MatrixApp();

	virtual void		MessageReceived(BMessage* msg);
			void		ImMessage(BMessage* msg);

			void		Connect();
			void		StartLoop();

			void		SendMessage(BMessage msg);
			void		SendError(const char* message,
							mtx::http::RequestErr err, bool fatal = false);

	BMessage* fSettings;
	status_t fInitStatus;
	BStringList fRoomList;

private:
	// Settings
	BString fUser;
	BString fPassword;
	BString fServer;
	BString fSession;

	thread_id fProtoThread;
};


void print_error(mtx::http::RequestErr err, const char* message = NULL);

void login_handler(const mtx::responses::Login &res, mtx::http::RequestErr err);

void initial_sync_handler(const mtx::responses::Sync &res, mtx::http::RequestErr err);
void sync_handler(const mtx::responses::Sync &res, mtx::http::RequestErr err);

void room_sync(mtx::responses::Rooms rooms);

#endif // _MATRIX_APP_H
