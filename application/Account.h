/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <Handler.h>
#include <Messenger.h>

#include "CayaProtocol.h"

class Account : public CayaProtocolMessengerInterface {
public:
						Account(BHandler* msgTarget);
	virtual				~Account();

	virtual	status_t	SendMessage(BMessage* message);

private:
			BMessenger	fMessenger;		
};

#endif	// _ACCOUNT_H
