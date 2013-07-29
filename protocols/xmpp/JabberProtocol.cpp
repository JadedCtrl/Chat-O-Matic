/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "JabberProtocol.h"

const char* kProtocolSignature = "jabber";
const char* kProtocolName = "Jabber";


JabberProtocol::JabberProtocol()
	: JabberHandler()
{
}


JabberProtocol::~JabberProtocol()
{
}


void
JabberProtocol::OverrideSettings()
{
}


BString
JabberProtocol::ComposeJID() const
{
	BString jid(fUsername);
	if (jid.FindLast("@") < 0)
		jid << "@" << fServer;
	jid << "/" << fResource;
	return jid;
}
