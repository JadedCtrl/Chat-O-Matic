/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <stdio.h>

#include <File.h>
#include <Message.h>
#include <Path.h>

#include "Account.h"
#include "Utils.h"


Account::Account(bigtime_t instanceId, ChatProtocol* cayap,
	const char* name, const char* addOnSignature, BHandler* target)
	:
	fIdentifier(instanceId),
	fName(name),
	fStatus(B_ERROR),
	fProtocol(cayap),
	fMessenger(target),
	fSettings(new BMessage())
{
	fProtocol->Init(this);

	// Find user's settings path
	BPath path = AccountPath(addOnSignature, fProtocol->Signature());
	if (path.InitCheck() == B_OK) {
		path.Append(name);

		fProtocol->SetName(name);
		fProtocol->SetAccountCachePath(AccountCachePath(name));
		fProtocol->SetAddOnCachePath(AddOnCachePath(addOnSignature));

		// Load settings file
		BFile file(path.Path(), B_READ_ONLY);
		if (fSettings->Unflatten(&file) == B_OK)
			if (fSettings->GetBool("disabled", false) == false)
				fStatus = fProtocol->UpdateSettings(fSettings);
			else
				fStatus = B_DONT_DO_THAT;
	}
}


Account::~Account()
{
	delete fSettings;
}


status_t
Account::InitCheck()
{
	return fStatus;
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
