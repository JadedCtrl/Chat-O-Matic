/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <BeBuild.h>

#include "FacebookProtocol.h"
#include "GoogleTalkProtocol.h"
#include "JabberProtocol.h"


extern "C" _EXPORT CayaProtocol* protocol_at(int32 i);
extern "C" _EXPORT int32 protocol_count();
extern "C" _EXPORT const char* signature();
extern "C" _EXPORT const char* friendly_signature();
extern "C" _EXPORT uint32 version();


CayaProtocol*
protocol_at(int32 i)
{
	switch(i) {
		case 0:
			return (CayaProtocol*)new JabberProtocol();
		case 1:
			return (CayaProtocol*)new FacebookProtocol();
		case 2:
			return (CayaProtocol*)new GoogleTalkProtocol();
	}
	return NULL;
}


int32
protocol_count()
{
	return 3;
}


const char*
signature()
{
	return "jabber";
}


const char*
friendly_signature()
{
	return "Jabber";
}


uint32
version()
{
	return CAYA_VERSION_1_PRE_ALPHA_1;
}


