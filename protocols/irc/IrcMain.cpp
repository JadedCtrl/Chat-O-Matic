/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IrcProtocol.h"


extern "C" _EXPORT ChatProtocol* protocol_at(int32 i);
extern "C" _EXPORT int32 protocol_count();
extern "C" _EXPORT const char* signature();
extern "C" _EXPORT const char* friendly_signature();
extern "C" _EXPORT uint32 version();


ChatProtocol*
protocol_at(int32 i)
{
	if (i == 0)
			return (ChatProtocol*)new IrcProtocol();
	return NULL;
}


int32
protocol_count()
{
	return 1;
}


const char*
signature()
{
	return "irc";
}


const char*
friendly_signature()
{
	return "IRC";
}


uint32
version()
{
	return APP_VERSION_1_ALPHA_1;
}


