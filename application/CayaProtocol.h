/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PROTOCOL_H
#define _CAYA_PROTOCOL_H

#include <Messenger.h>

class BBitmap;


// Caya protocol interface version
#define CAYA_VERSION_1_PRE_ALPHA_1		0x00000001
#define CAYA_VERSION_1_ALPHA_1			0x00000100

#define CAYA_VERSION 					CAYA_VERSION_1_PRE_ALPHA_1


class CayaProtocolMessengerInterface {
public:
	virtual status_t SendMessage(BMessage* message) = 0;
};

class CayaProtocol {
public:
	//! Messenger
	virtual status_t Init(CayaProtocolMessengerInterface*) = 0;

	//! Called before unloading from memory
	virtual status_t Shutdown() = 0;

	//! Process message
	virtual status_t Process(BMessage*) = 0;

	//! Change settings
	virtual status_t UpdateSettings(BMessage*) = 0;

	//! Return a settings template
	//	Currently there are two: "account" (used when creating/editing
	//	the user's account) and "room" (used when creating a room).
	virtual BMessage SettingsTemplate(const char* name) = 0;

	//! Protocol signature
	virtual const char* Signature() const = 0;

	//! Protocol name
	virtual const char* FriendlySignature() const = 0;

	//! Protocol icon
	virtual BBitmap* Icon() const = 0;

	//! Add-on's path
	virtual void SetAddOnPath(BPath path) = 0;
	virtual BPath AddOnPath() = 0;

	//! Name of account file (leaf)
	virtual const char* GetName() = 0;
	virtual void SetName(const char* name) = 0;

	//! Preferred encoding of messages
	virtual uint32 GetEncoding() = 0;

	//! Messenger interface used
	virtual CayaProtocolMessengerInterface* MessengerInterface() const = 0;
};

#endif	// _CAYA_PROTOCOL_H
