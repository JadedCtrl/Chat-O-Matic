/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IrcProtocol.h"

#include <iostream>

#include <Catalog.h>
#include <Resources.h>
#include <SecureSocket.h>
#include <Socket.h>

#include <ChatProtocolMessages.h>

#include "Numerics.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IrcProtocol"


status_t
connect_thread(void* data)
{
	IrcProtocol* protocol = (IrcProtocol*)data;
	status_t status = protocol->Loop();
	exit(status);
}


IrcProtocol::IrcProtocol()
	:
	fSocket(NULL),
	fNick(NULL),
	fIdent(NULL)
{
}


IrcProtocol::~IrcProtocol()
{
	Shutdown();
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
	BString cmd = "QUIT";
	cmd << " :" << fPartText << "\n";
	_SendIrc(cmd);

	kill_thread(fRecvThread);
	return B_OK;
}


status_t
IrcProtocol::UpdateSettings(BMessage* settings)
{
	fNick = settings->FindString("nick");
	fPartText = settings->GetString("part", "Cardie[0.1]: i've been liquified!");
	const char* ident = settings->FindString("ident");
	const char* real_name = settings->FindString("real_name");
	const char* server = settings->FindString("server");
	const char* password = settings->FindString("password");
	int32 port = settings->FindInt32("port");
	bool ssl = settings->GetBool("ssl", false);

	fSocket = ssl ? new BSecureSocket : new BSocket;

	if (fSocket->Connect(BNetworkAddress(server, port)) != B_OK)
		return B_ERROR;

	if (password != NULL) {
		BString passMsg = "PASS ";
		passMsg << password << "\n";
		_SendIrc(passMsg);
	}

	BString userMsg = "USER ";
	userMsg << ident << " * 0 :" << real_name << "\n";
	_SendIrc(userMsg);

	BString nickMsg = "NICK ";
	nickMsg << fNick << "\n";
	_SendIrc(nickMsg);

	fRecvThread = spawn_thread(connect_thread, "what_a_tangled_web_we_weave",
		B_NORMAL_PRIORITY, (void*)this);

	if (fRecvThread < B_OK)
		return B_ERROR;

	resume_thread(fRecvThread);
	return B_OK;
}


status_t
IrcProtocol::Process(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");
	switch (im_what) {
		case IM_JOIN_ROOM:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) == B_OK) {
				BString cmd = "JOIN ";
				cmd << chat_id << "\n";
				_SendIrc(cmd);
			}
			break;
		}
		default:
			std::cout << "Unhandled message for IRC:\n";
			msg->PrintToStream();
	}
	return B_ERROR;
}


BMessage
IrcProtocol::SettingsTemplate(const char* name)
{
	BMessage settings;
	if (strcmp(name, "account") == 0)
		settings = _AccountTemplate();
	else if (strcmp(name, "join_room") == 0 || strcmp(name, "create_room") == 0)
		settings = _RoomTemplate();
	return settings;
}


status_t
IrcProtocol::Loop()
{
	while (fSocket != NULL && fSocket->IsConnected() == true)
		_ProcessLine(_ReadUntilNewline(fSocket, &fRemainingBuf));
	return B_OK;
}


void
IrcProtocol::_ProcessLine(BString line)
{
	BStringList words;
	line.Split(" ", true, words);
	BString sender = _LineSender(words);
	BString code = _LineCode(words);
	BStringList params = _LineParameters(words);
	BString body = _LineBody(line);

	int32 numeric;
	if ((numeric = atoi(code.String())) > 0)
		_ProcessNumeric(numeric, sender, params, body);
	else
		_ProcessCommand(code, sender, params, body);
}


void
IrcProtocol::_ProcessNumeric(int32 numeric, BString sender, BStringList params,
	BString body)
{
	switch (numeric) {
		case RPL_WELCOME:
		{
			BMessage ready(IM_MESSAGE);
			ready.AddInt32("im_what", IM_PROTOCOL_READY);
			_SendMsg(&ready);

			BMessage self(IM_MESSAGE);
			self.AddInt32("im_what", IM_OWN_CONTACT_INFO);
			self.AddString("user_id", fNick);
			_SendMsg(&self);
			break;
		}
		case RPL_MOTD:
		{
			BMessage send(IM_MESSAGE);
			send.AddInt32("im_what", IM_MESSAGE_RECEIVED);
			send.AddString("chat_id", sender);
			send.AddString("body", body);
			_SendMsg(&send);
			break;
		}
		case ERR_NICKNAMEINUSE:
		{
			fNick << "_";
			BString cmd("NICK ");
			cmd << fNick << "\n";
			_SendIrc(cmd);
			break;
		}
	}
}


void
IrcProtocol::_ProcessCommand(BString command, BString sender,
	BStringList params, BString body)
{
	if (command == "PING") {
		BString cmd = "PONG ";
		cmd << body.RemoveChars(0, 1) << "\n";
		_SendIrc(cmd);
	}
}


BString
IrcProtocol::_LineSender(BStringList words)
{
	BString sender;
	if (words.CountStrings() > 1)
		sender = words.StringAt(0).RemoveChars(0, 1);
	return sender;
}


BString
IrcProtocol::_LineCode(BStringList words)
{
	BString code;
	if (words.CountStrings() > 2)
		code = words.StringAt(1);
	return code;
}


BStringList
IrcProtocol::_LineParameters(BStringList words)
{
	BStringList params;
	BString current;
	for (int i = 2; i < words.CountStrings(); i++)
		if ((current = words.StringAt(i)).StartsWith(":") == false)
			params.Add(current);
		else
			break;
	return params;
}


BString
IrcProtocol::_LineBody(BString line)
{
	BString body;
	int32 index = line.RemoveChars(0, 1).FindFirst(":");
	if (index != B_ERROR)
		body = line.RemoveChars(0, index + 1);
	return body;
}


void
IrcProtocol::_SendMsg(BMessage* msg)
{
	fMessenger->SendMessage(msg);
}


void
IrcProtocol::_SendIrc(BString cmd)
{
	if (fSocket != NULL && fSocket->IsConnected() == true)
		fSocket->Write(cmd.String(), cmd.CountChars());
	else {
		BMessage disable(IM_MESSAGE);
		disable.AddInt32("im_what", IM_PROTOCOL_DISABLE);
	}
}


BString
IrcProtocol::_ReadUntilNewline(BDataIO* io, BString* extraBuffer)
{
	BString total;
	char buf[1024] = { '\0' };

	// Use buffer from last read if any text remains
	if (extraBuffer->IsEmpty() == false) {
		BString trimRet = _TrimStringToNewline(extraBuffer);
		if (trimRet.IsEmpty() == true)
			total << extraBuffer;
		else
			return trimRet;
	}

	while (!(strstr(buf, "\n"))) {
		io->Read(buf, 1023);
		std::cerr << buf << std::endl;
		total << buf;
	}

	BString currentLine = _TrimStringToNewline(&total);
	extraBuffer->SetTo(total);
	return currentLine;
}


BString
IrcProtocol::_TrimStringToNewline(BString* str)
{
	BString line;
	int32 lineEnd = str->FindFirst('\n');

	if (lineEnd != B_ERROR) {
		str->CopyCharsInto(line, 0, lineEnd + 1);
		str->RemoveChars(0, lineEnd + 1);
	}
	return line;
}


BMessage
IrcProtocol::_AccountTemplate()
{
	BMessage settings;

	BMessage server;
	server.AddString("name", "server");
	server.AddString("description", B_TRANSLATE("Server:"));
	server.AddString("default", "irc.oftc.net");
	server.AddString("error", B_TRANSLATE("Please enter a valid server address."));
	server.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &server);

	BMessage port;
	port.AddString("name", "port");
	port.AddString("description", B_TRANSLATE("Port:"));
	port.AddInt32("default", 6697);
	port.AddString("error", B_TRANSLATE("We need a port-number to know which door to knock on! Likely 6667/6697."));
	port.AddInt32("type", B_INT32_TYPE);
	settings.AddMessage("setting", &port);

	BMessage ssl;
	ssl.AddString("name", "ssl");
	ssl.AddString("description", B_TRANSLATE("SSL:"));
	ssl.AddBool("default", true);
	ssl.AddInt32("type", B_BOOL_TYPE);
	settings.AddMessage("setting", &ssl);

	BMessage nick;
	nick.AddString("name", "nick");
	nick.AddString("description", B_TRANSLATE("Nickname:"));
	nick.AddString("default", "Haikunaut");
	nick.AddString("error", B_TRANSLATE("You need a default nickname― The Nameless are not welcome on IRC."));
	nick.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &nick);

	BMessage ident;
	ident.AddString("name", "ident");
	ident.AddString("description", B_TRANSLATE("Ident:"));
	ident.AddString("error", B_TRANSLATE("You need a username in order to connect!"));
	ident.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &ident);

	BMessage password;
	password.AddString("name", "password");
	password.AddString("description", B_TRANSLATE("Password:"));
	password.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &password);

	BMessage realName;
	realName.AddString("name", "real_name");
	realName.AddString("description", B_TRANSLATE("Real name:"));
	realName.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &realName);

	BMessage part;
	part.AddString("name", "part");
	part.AddString("description", B_TRANSLATE("Part message:"));
	part.AddInt32("type", B_STRING_TYPE);
	part.AddString("default", "Cardie[0.1]: i've been liquified!");
	settings.AddMessage("setting", &part);

	return settings;
}


BMessage
IrcProtocol::_RoomTemplate()
{
	BMessage settings;

	BMessage id;
	id.AddString("name", "chat_id");
	id.AddString("description", B_TRANSLATE("Channel:"));
	id.AddString("error", B_TRANSLATE("Please enter a channel― skipping it doesn't make sense!"));
	id.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &id);

	return settings;
}
