/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <String.h>

#include "LooperCayaProtocol.h"


LooperCayaProtocol::LooperCayaProtocol(CayaProtocol* protocol)
	: BLooper(),
	fProtocol(protocol)
{
	BString name("CayaProcol - ");
	name << protocol->GetFriendlySignature();
	SetName(name.String());
	Run();
}


void	
LooperCayaProtocol::MessageReceived(BMessage* msg)
{
	if (Protocol()->Process(msg) != B_OK)
		BLooper::MessageReceived(msg);	
}


CayaProtocol*
LooperCayaProtocol::Protocol()
{
	return fProtocol;
}
