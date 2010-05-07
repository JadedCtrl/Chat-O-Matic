/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef CayaProtocol_h
#define CayaProtocol_h

#include <Messenger.h>

class CayaProtocolMessengerInterface {
	
	public:
		virtual status_t SendMessage(BMessage *message) = 0;
								
};

class CayaProtocol
{
	public:
		
		// messenger
		virtual status_t Init( CayaProtocolMessengerInterface * ) = 0;

		// called before unloading from memory
		virtual status_t Shutdown() = 0;

		// process message
		virtual status_t Process( BMessage * ) = 0;
		
		// Get name of protocol
		virtual const char * GetSignature() = 0;
		virtual const char * GetFriendlySignature() = 0;

		// settings changed
		virtual status_t UpdateSettings( BMessage & ) = 0;

		// preferred encoding of messages
		virtual uint32 GetEncoding() = 0;
};


#endif
