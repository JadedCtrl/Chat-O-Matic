/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_H
#define _ACCOUNT_H

#include <Handler.h>
#include <Messenger.h>
#include <String.h>

#include "CayaProtocol.h"

class Account : public CayaProtocolMessengerInterface {
public:
							Account(bigtime_t instanceId, CayaProtocol* cayap,
									const char* name, BHandler* target);
	virtual					~Account();

			bigtime_t		Identifier() const;
			const char*		Name() const;

	virtual	status_t		SendMessage(BMessage* message);

private:
			bigtime_t		fIdentifier;
			CayaProtocol*	fProtocol;
			BString			fName;
			BMessenger		fMessenger;
			BMessage*		fSettings;
};

#endif	// _ACCOUNT_H
