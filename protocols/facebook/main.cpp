#include "Facebook.h"

extern "C" __declspec(dllexport) CayaProtocol* protocol();
extern "C" __declspec(dllexport) const char* signature();
extern "C" __declspec(dllexport) const char* friendly_signature();

CayaProtocol*
protocol()
{
	return (CayaProtocol*)new Facebook();
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
