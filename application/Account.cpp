/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "Account.h"


Account::Account(BHandler* msgTarget)
{
	BMessenger msgr(msgTarget);
	fMessenger = msgr;
}


Account::~Account()
{
}


status_t 
Account::SendMessage(BMessage* message)
{
	 // This is just an example of what can be done ;)
	 message->AddPointer("account", (void*)this);	
	 return fMessenger.SendMessage(message); 
}
