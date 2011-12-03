/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PROTOCOL_H
#define _CAYA_PROTOCOL_H

#include <Messenger.h>

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

	//! Protocol signature
	virtual const char* Signature() const = 0;

	//! Protocol name
	virtual const char* FriendlySignature() const = 0;

	//! Preferred encoding of messages
	virtual uint32 GetEncoding() = 0;

	//! Messenger interface used
	virtual CayaProtocolMessengerInterface* MessengerInterface() const = 0;

	//! Caya interface version
	virtual uint32 Version() const = 0;
};

#endif	// _CAYA_PROTOCOL_H
