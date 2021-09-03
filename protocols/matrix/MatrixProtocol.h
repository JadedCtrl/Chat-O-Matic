/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _MATRIX_PROTOCOL_H
#define _MATRIX_PROTOCOL_H

#include <String.h>

#include <ChatProtocol.h>

#include "Matrix.h"


status_t connect_thread(void* data);
BMessage receive_message();


class MatrixProtocol : public ChatProtocol {
public:
						MatrixProtocol();
						~MatrixProtocol();

	// ChatProtocol inheritance
	virtual	status_t	Init(ChatProtocolMessengerInterface* interface);
	virtual	status_t	Shutdown();

	virtual	status_t	UpdateSettings(BMessage* settings);

	virtual	status_t	Process(BMessage* msg);

	virtual	BMessage	SettingsTemplate(const char* name);
	virtual BObjectList<BMessage>
						Commands();

	virtual	const char*	Signature() const { return MATRIX_ADDON; }
	virtual const char*	FriendlySignature() const { return "Matrix"; }

	virtual BBitmap*	Icon() const;

	virtual	void		SetAddOnPath(BPath path) { fAddOnPath = path; }
	virtual	BPath		AddOnPath() { return fAddOnPath; }

	virtual	const char*	GetName() { return fName; }
	virtual	void		SetName(const char* name) { fName = name; }

	virtual	ChatProtocolMessengerInterface*
						MessengerInterface() const { return fMessenger; }

	// Matrix
			void		SendMessage(BMessage* msg);
			void		RegisterApp(team_id team);

private:
			void		_SendMatrixMessage(BMessage* msg);
			void		_StartApp();

			// GUI templates
			BMessage	_AccountTemplate();
			BMessage	_RoomTemplate();
			BMessage	_RosterTemplate();

	// Settings
	BMessage* fSettings;

	BPath fAddOnPath;
	BString fName;
	ChatProtocolMessengerInterface* fMessenger;

	team_id fAppTeam;
	thread_id fRecvThread;
	BMessenger* fAppMessenger;
};

#endif // _MATRIX_PROTOCOL_H
