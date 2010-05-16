/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_LOOPER_H
#define _PROTOCOL_LOOPER_H

#include <Looper.h>

#include "CayaProtocol.h"

class ProtocolLooper : public BLooper {
public:		
					ProtocolLooper(CayaProtocol* protocol);

			void	MessageReceived(BMessage* msg);

	CayaProtocol*	Protocol();

private:
	CayaProtocol*	fProtocol;
};

#endif	// _PROTOCOL_LOOPER_H
