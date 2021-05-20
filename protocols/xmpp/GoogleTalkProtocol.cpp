/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "GoogleTalkProtocol.h"

const char* kProtocolSignature = "gtalk";
const char* kProtocolName = "GoogleTalk";


GoogleTalkProtocol::GoogleTalkProtocol()
	: JabberHandler()
{
}


GoogleTalkProtocol::~GoogleTalkProtocol()
{
}


void
GoogleTalkProtocol::OverrideSettings()
{
	fServer = "talk.google.com";
	fPort = 0;
}


BMessage
GoogleTalkProtocol::SettingsTemplate()
{
	return JabberHandler::_SettingsTemplate("Identifier", false);
}


BString
GoogleTalkProtocol::ComposeJID() const
{
	BString jid(fUsername);
	if (jid.FindLast("@") < 0)
		jid << "@gmail.com";
	jid << "/" << fResource;
	return jid;
}
