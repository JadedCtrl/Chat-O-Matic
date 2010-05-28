/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "GoogleTalkProtocol.h"

extern "C" __declspec(dllexport) CayaProtocol* protocol();
extern "C" __declspec(dllexport) const char* signature();
extern "C" __declspec(dllexport) const char* friendly_signature();

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
