/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "FacebookProtocol.h"

#include <Resources.h>

#include <libinterface/BitmapUtils.h>


FacebookProtocol::FacebookProtocol()
	: JabberHandler()
{
}


FacebookProtocol::~FacebookProtocol()
{
}


const char*
FacebookProtocol::Signature() const
{
	return "facebook";
}


const char*
FacebookProtocol::FriendlySignature() const
{
	return "Facebook";
}


BBitmap*
FacebookProtocol::Icon() const
{
	BResources res(fPath.Path());
	return IconFromResources(&res, 1, B_LARGE_ICON);
}


BMessage
FacebookProtocol::SettingsTemplate(const char* name)
{
	if (name == BString("account"))
		return JabberHandler::_SettingsTemplate("Username", false);
	else
		return BMessage();
}


void
FacebookProtocol::OverrideSettings()
{
	fServer = "chat.facebook.com";
}


BString
FacebookProtocol::ComposeJID() const
{
	BString jid(fUsername);
	jid << "@" << fServer << "/" << fResource;
	return jid;
}
