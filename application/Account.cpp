/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <File.h>
#include <Message.h>
#include <Path.h>

#include "Account.h"
#include "CayaUtils.h"


Account::Account(bigtime_t instanceId, CayaProtocol* cayap,
	 const char* name, BHandler* target)
	:
	fIdentifier(instanceId),
	fName(name),
	fProtocol(cayap),
	fMessenger(target),
	fSettings(new BMessage())
{
	fProtocol->Init(this);

	// Find user's settings path
	BPath path(CayaAccountPath(fProtocol->Signature()));
	if (path.InitCheck() == B_OK) {
		path.Append(name);

		// Load settings file
		BFile file(path.Path(), B_READ_ONLY);
		if (fSettings->Unflatten(&file) == B_OK)
			fProtocol->UpdateSettings(fSettings);
	}
}


Account::~Account()
{
	delete fSettings;
}


bigtime_t
Account::Identifier() const
{
	return fIdentifier;
}


const char*
Account::Name() const
{
	return fName.String();
}


status_t
Account::SendMessage(BMessage* message)
{
	 message->AddInt64("instance", fIdentifier);
	 return fMessenger.SendMessage(message);
}
