/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "GoogleTalkProtocol.h"

#include <Resources.h>

#include <libinterface/BitmapUtils.h>


GoogleTalkProtocol::GoogleTalkProtocol()
	: JabberHandler()
{
}


GoogleTalkProtocol::~GoogleTalkProtocol()
{
}


const char*
GoogleTalkProtocol::Signature() const
{
	return "gtalk";
}


const char*
GoogleTalkProtocol::FriendlySignature() const
{
	return "GTalk";
}


BBitmap*
GoogleTalkProtocol::Icon() const
{
	BResources res(fPath.Path());
	return IconFromResources(&res, 2, B_LARGE_ICON);
}


BMessage
GoogleTalkProtocol::SettingsTemplate(const char* name)
{
	if (name == BString("account"))
		return JabberHandler::_SettingsTemplate("Identifier", false);
	else
		return BMessage();
}


void
GoogleTalkProtocol::OverrideSettings()
{
	fServer = "dismail.de";
}


BString
GoogleTalkProtocol::ComposeJID() const
{
	BString jid(fUsername);
	if (jid.FindLast("@") < 0)
		jid << "@dismail.de";
	jid << "/" << fResource;
	return jid;
}
