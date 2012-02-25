/*
 * Copyright 2010, Alexander Botero-Lowry. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "AIM.h"

extern "C" {
	CayaProtocol* protocol();
	const char* signature();
	const char* friendly_signature();
};

CayaProtocol*
protocol()
{
	return (CayaProtocol*)new AIMProtocol();
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
