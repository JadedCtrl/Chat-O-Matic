/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IrcProtocol.h"

#include <iostream>


status_t
connect_thread(void* data)
{
	BMessage* settings = (BMessage*)data;
	if (!settings)
		return B_ERROR;

	const char* nick = settings->FindString("nick");
	const char* ident = settings->FindString("ident");
	const char* real_name = settings->FindString("real_name");
	const char* server = settings->FindString("server");
	const char* password = settings->FindString("password");
	int32 port = settings->FindInt32("port");
	bool ssl = false;

	if (!nick || !ident || !server)
		return B_ERROR;

	// libircclient wants a "#" in front of SSL addresses
	BString joinServer;
	if (ssl == true)
		joinServer << "#";
	joinServer << server;

	settings->PrintToStream();

	// Now create the session
	irc_callbacks_t callbacks = get_callbacks();
	irc_session_t* session = irc_create_session(&callbacks);

	if (!session)
		return B_ERROR;

	// Start connection
	if (irc_connect(session, joinServer.String(), port, password, nick, ident,
		real_name))
	{
		printf("Could not connect: %s\n", irc_strerror (irc_errno(session)));
		return B_ERROR;
	}

	// Start network loop
	if (irc_run(session)) {
		printf("Could not connect or I/O error: %s\n", irc_strerror
			(irc_errno(session)));
		return B_ERROR;
	}
	return B_OK;
}


IrcProtocol::IrcProtocol()
{
}


status_t
IrcProtocol::Init(ChatProtocolMessengerInterface* interface)
{
	fMessenger = interface;
	return B_OK;
}


status_t
IrcProtocol::Shutdown()
{
	return B_OK;
}


status_t
IrcProtocol::Process(BMessage* msg)
{
	return B_OK;
}


status_t
IrcProtocol::UpdateSettings(BMessage* msg)
{
	fServerThread = spawn_thread(connect_thread, "connect_thread",
		B_NORMAL_PRIORITY, (void*)msg);

	if (fServerThread < B_OK)
		return B_ERROR;

	resume_thread(fServerThread);
	return B_OK;
}


BMessage
IrcProtocol::SettingsTemplate(const char* name)
{
	size_t size;
	BMessage temp;
	const void* buff = fResources.LoadResource(B_MESSAGE_TYPE, name, &size);

	if (buff != NULL)
		temp.Unflatten((const char*)buff);
	return temp;
}


BObjectList<BMessage>
IrcProtocol::Commands()
{
	return BObjectList<BMessage>();
}


BObjectList<BMessage>
IrcProtocol::UserPopUpItems()
{
	return BObjectList<BMessage>();
}


BObjectList<BMessage>
IrcProtocol::ChatPopUpItems()
{
	return BObjectList<BMessage>();
}

BObjectList<BMessage>
IrcProtocol::MenuBarItems()
{
	return BObjectList<BMessage>();
}


const char*
IrcProtocol::Signature() const
{
	return "irc";
}


const char*
IrcProtocol::FriendlySignature() const
{
	return "IRC";
}


BBitmap*
IrcProtocol::Icon() const
{
	return NULL;
}


void
IrcProtocol::SetAddOnPath(BPath path)
{
	fAddOnPath = path;
	fResources.SetTo(path.Path());
}


BPath
IrcProtocol::AddOnPath()
{
	return fAddOnPath;
}


const char*
IrcProtocol::GetName()
{
	return fName.String();
}


void
IrcProtocol::SetName(const char* name)
{
	fName.SetTo(name);
}


uint32
IrcProtocol::GetEncoding()
{
	return 0xffff;
}


ChatProtocolMessengerInterface*
IrcProtocol::MessengerInterface() const
{
	return fMessenger;
}


irc_callbacks_t
get_callbacks()
{
	irc_callbacks_t callbacks;
	callbacks.event_connect = event_connect;
	callbacks.event_numeric = event_numeric;
	callbacks.event_join = event_join;
	return callbacks;
}


void
event_connect(irc_session_t* session, const char* event,
	const char* origin, const char** params, unsigned int count)
{
}


void
event_numeric(irc_session_t* session, unsigned int blah,
	const char* origin, const char** params, unsigned int count)
{
}


void
event_join(irc_session_t* session, const char* event, const char* origin,
	const char** params, unsigned int count)
{
}
