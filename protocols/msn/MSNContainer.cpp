/*
 * Copyright 2010 Your Name <your@email.address>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#include <string.h>

#include "MSNContainer.h"

MSNContainer::MSNContainer(string buddy)
	:
	fMessage(""),
	fRCPT(buddy),
	fMSNObject(""),
	fIfMsg(false)
{

}


MSNContainer::MSNContainer(string msg, string rcpt)
	:
	fMessage(msg),
	fRCPT(rcpt),
	fMSNObject(""),
	fIfMsg(true)
{
}


MSNContainer::~MSNContainer()
{
	
	
}
