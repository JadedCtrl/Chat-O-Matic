#include "MSN.h"

extern "C" {
	CayaProtocol* protocol();
	const char* signature();
	const char* friendly_signature();
}

CayaProtocol*
protocol()
{
	return (CayaProtocol*)new MSNP();
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
