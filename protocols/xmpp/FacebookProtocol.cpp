/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "FacebookProtocol.h"

const char* kProtocolSignature = "facebook";
const char* kProtocolName = "Facebook";


FacebookProtocol::FacebookProtocol()
	: JabberHandler()
{
}


FacebookProtocol::~FacebookProtocol()
{
}


void
FacebookProtocol::OverrideSettings()
{
	fServer = "chat.facebook.com";
}


BMessage
FacebookProtocol::SettingsTemplate()
{
	return JabberHandler::_SettingsTemplate("Username", false);
}


BString
FacebookProtocol::ComposeJID() const
{
	BString jid(fUsername);
	jid << "@" << fServer << "/" << fResource;
	return jid;
}
