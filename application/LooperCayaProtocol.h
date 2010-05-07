/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef LooperCayaProtocol_h
#define LooperCayaProtocol_h

#include <Looper.h>
#include "CayaProtocol.h"

class LooperCayaProtocol : public BLooper
{
	public:		
					LooperCayaProtocol(CayaProtocol* protocol);

			void	MessageReceived(BMessage* msg);
			
			CayaProtocol*	Protocol();
					
	private:
			CayaProtocol*	fProtocol;
};


#endif
