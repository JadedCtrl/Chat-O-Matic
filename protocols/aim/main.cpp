/*
 * Copyright 2010, Alexander Botero-Lowry. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "AIM.h"

extern "C" __declspec(dllexport) CayaProtocol *main_protocol();

CayaProtocol *main_protocol()
{
	return (CayaProtocol*)new AIMProtocol();
}
