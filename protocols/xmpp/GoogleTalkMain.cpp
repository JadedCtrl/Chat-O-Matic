/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <BeBuild.h>

#include "GoogleTalkProtocol.h"

extern "C" _EXPORT CayaProtocol* protocol();
extern "C" _EXPORT const char* signature();
extern "C" _EXPORT const char* friendly_signature();


CayaProtocol*
protocol()
{
	return (CayaProtocol*)new GoogleTalkProtocol();
}


const char*
signature()
{
	return kProtocolSignature;
}


const char*
friendly_signature()
{
	return kProtocolName;
}
