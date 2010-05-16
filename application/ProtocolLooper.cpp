/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <String.h>

#include "Account.h"
#include "ProtocolLooper.h"


ProtocolLooper::ProtocolLooper(CayaProtocol* protocol)
	: BLooper(),
	fProtocol(protocol)
{
	Account* account = reinterpret_cast<Account*>(
		protocol->MessengerInterface());

	BString name(protocol->FriendlySignature());
	name << " - " << account->Name();

	SetName(name.String());
	Run();
}


void	
ProtocolLooper::MessageReceived(BMessage* msg)
{
	if (Protocol()->Process(msg) != B_OK)
		BLooper::MessageReceived(msg);
}


CayaProtocol*
ProtocolLooper::Protocol()
{
	return fProtocol;
}
