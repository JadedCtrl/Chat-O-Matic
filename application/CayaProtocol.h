/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PROTOCOL_H
#define _CAYA_PROTOCOL_H

#include <Messenger.h>

class CayaProtocolMessengerInterface {
public:
	virtual status_t SendMessage(BMessage* message) = 0;
};

class CayaProtocol {
public:
	// Messenger
	virtual status_t Init(CayaProtocolMessengerInterface*) = 0;

	// Called before unloading from memory
	virtual status_t Shutdown() = 0;

	// Process message
	virtual status_t Process(BMessage*) = 0;

	// Change settings
	virtual status_t UpdateSettings(BMessage*) = 0;

	// Protocol information
	virtual const char* Signature() const = 0;
	virtual const char* FriendlySignature() const = 0;

	// Preferred encoding of messages
	virtual uint32 GetEncoding() = 0;

	// Messenger interface used
	virtual CayaProtocolMessengerInterface* MessengerInterface() const = 0;
};

#endif	// _CAYA_PROTOCOL_H
