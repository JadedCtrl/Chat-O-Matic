#include "GoogleTalk.h"

extern "C" __declspec(dllexport) CayaProtocol *main_protocol ();

CayaProtocol *main_protocol ()
{
	return (CayaProtocol *)(new GoogleTalk());
}
