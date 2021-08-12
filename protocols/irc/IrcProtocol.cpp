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

#include <libinterface/BitmapUtils.h>

#include <AppConstants.h>
#include <ChatProtocolMessages.h>
#include <Flags.h>

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
	fIdent(NULL),
	fReady(false),
	fWriteLocked(false)
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
	BString cmd = "QUIT :";
	cmd << fPartText;
	_SendIrc(cmd);

	kill_thread(fRecvThread);
	return B_OK;
}


status_t
IrcProtocol::UpdateSettings(BMessage* settings)
{
	fNick = settings->FindString("nick");
	fPartText = settings->GetString("part", "Cardie[0.1]: i've been liquified!");
	fUser = settings->FindString("ident");
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
		passMsg << password;
		_SendIrc(passMsg);
	}

	BString userMsg = "USER ";
	userMsg << fUser << " * 0 :" << real_name;
	_SendIrc(userMsg);

	BString nickMsg = "NICK ";
	nickMsg << fNick;
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
		case IM_SEND_MESSAGE:
		{
			BString chat_id = msg->FindString("chat_id");
			BString body = msg->FindString("body");
			if (chat_id.IsEmpty() == false || body.IsEmpty() == false) {
				BStringList lines;
				body.Split("\n", true, lines);

				for (int i = 0; i < lines.CountStrings(); i++) {
					BString cmd = "PRIVMSG ";
					cmd << chat_id << " :" << lines.StringAt(i);
					_SendIrc(cmd);

					BMessage sent(IM_MESSAGE);
					sent.AddInt32("im_what", IM_MESSAGE_SENT);
					sent.AddString("user_id", fIdent);
					sent.AddString("chat_id", chat_id);
					sent.AddString("body", lines.StringAt(i));
					_SendMsg(&sent);
				}
			}
			break;
		}
		case IM_ROOM_INVITE_ACCEPT:
		case IM_JOIN_ROOM:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) == B_OK) {
				BString cmd = "JOIN ";
				cmd << chat_id;
				_SendIrc(cmd);
			}
			break;
		}
		case IM_LEAVE_ROOM:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) == B_OK) {
				if (_IsChannelName(chat_id) == true) {
					BString cmd = "PART ";
					cmd << chat_id << " * :" << fPartText;
					_SendIrc(cmd);
				}
				else {
					BMessage left(IM_MESSAGE);
					left.AddInt32("im_what", IM_ROOM_LEFT);
					left.AddString("chat_id", chat_id);
					_SendMsg(&left);
				}
			}
			break;
		}
		case IM_GET_ROOM_METADATA:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) == B_OK) {
				BMessage meta(IM_MESSAGE);
				meta.AddInt32("im_what", IM_ROOM_METADATA);
				meta.AddString("chat_id", chat_id);
				meta.AddInt32("room_default_flags",
					ROOM_LOG_LOCALLY | ROOM_POPULATE_LOGS);
				_SendMsg(&meta);
			}
			break;
		}
		case IM_ROOM_SEND_INVITE:
		{
			BString chat_id = msg->FindString("chat_id");
			BString user_id = msg->FindString("user_id");
			if (chat_id.IsEmpty() == false || user_id.IsEmpty() == false) {
				BString cmd("INVITE ");
				cmd << _IdentNick(user_id) << " " << chat_id;
				_SendIrc(cmd);
			}
			break;
		}
		case IM_SET_OWN_NICKNAME:
		{
			BString user_name;
			if (msg->FindString("user_name", &user_name) == B_OK) {
				BString cmd("NICK ");
				cmd << user_name;
				_SendIrc(cmd);
			}
			break;
		}
		default:
			std::cout << "Unhandled message for IRC:\n";
			msg->PrintToStream();
			return B_ERROR;
	}
	return B_OK;
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


BBitmap*
IrcProtocol::Icon() const
{
	return ReadNodeIcon(fAddOnPath.Path(), B_LARGE_ICON, true);
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
	line.RemoveCharsSet("\n\r");
	line.Split(" ", true, words);
	BString sender = _LineSender(words);
	BString code = _LineCode(words);
	BStringList params = _LineParameters(words, line);

	int32 numeric;
	if ((numeric = atoi(code.String())) > 0)
		_ProcessNumeric(numeric, sender, params, line);
	else
		_ProcessCommand(code, sender, params, line);
}


void
IrcProtocol::_ProcessNumeric(int32 numeric, BString sender, BStringList params,
	BString line)
{
	if (numeric > 400) {
		_ProcessNumericError(numeric, sender, params, line);
		return;
	}

	switch (numeric) {
		case RPL_WELCOME:
		{
			if (params.CountStrings() == 2)
				fNick = params.First();
			BString cmd("WHO ");
			cmd << fNick << "\n";
			_SendIrc(cmd);
			break;
		}
		case RPL_WHOREPLY:
		{
			BString channel = params.StringAt(1);
			BString user = params.StringAt(2);
			BString host = params.StringAt(3);
			BString nick = params.StringAt(5);
			BString ident = user;
			ident << "@" << host;

			// Contains the user's contact info― protocol ready!
			if (fReady == false && nick == fNick) {
				fUser = user.String();
				_MakeReady(nick, ident);
			}

			// Used to populate a room's userlist
			if (fWhoRequested == false && channel != "*") {
				BMessage user(IM_MESSAGE);
				user.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
				user.AddString("chat_id", channel);
				user.AddString("user_id", ident);
				user.AddString("user_name", nick);
				fIdentNicks.AddItem(ident, nick);
				_SendMsg(&user);
			}
			break;
		}
		case RPL_ENDOFWHO:
			fWhoRequested = false;
			break;
		case RPL_TOPIC:
		{
			BString chat_id = params.StringAt(1);
			BString subject = params.Last();

			BMessage topic(IM_MESSAGE);
			topic.AddInt32("im_what", IM_ROOM_SUBJECT_SET);
			topic.AddString("subject", subject);
			topic.AddString("chat_id", chat_id);
			_SendMsg(&topic);
			break;
		}
		case RPL_MOTDSTART:
		case RPL_MOTD:
		case RPL_ENDOFMOTD:
		{
			BString body = params.Last();
			if (numeric == RPL_MOTDSTART)
				body = "――MOTD start――";
			else if (numeric == RPL_ENDOFMOTD)
				body = "――MOTD end――";
			BMessage send(IM_MESSAGE);
			send.AddInt32("im_what", IM_MESSAGE_RECEIVED);
			send.AddString("body", body);
			_SendMsg(&send);
			break;
		}
	}
}


void
IrcProtocol::_ProcessNumericError(int32 numeric, BString sender,
	BStringList params, BString line)
{
	switch (numeric) {
		case ERR_NICKNAMEINUSE:
		{
			fNick << "_";
			BString cmd("NICK ");
			cmd << fNick << "\n";
			_SendIrc(cmd);
			break;
		}
		default:
		{
			BMessage err(IM_MESSAGE);
			err.AddInt32("im_what", IM_MESSAGE_RECEIVED);
			err.AddString("body", line);
			_SendMsg(&err);
		}
	}
}


void
IrcProtocol::_ProcessCommand(BString command, BString sender,
	BStringList params, BString line)
{
	// If protocol uninitialized and the user's ident is mentioned― use it!
	if (fReady == false && _SenderNick(sender) == fNick)
		_MakeReady(_SenderNick(sender), _SenderIdent(sender));

	if (sender == "PING")
	{
		BString cmd = "PONG ";
		cmd << params.Last() << "\n";
		_SendIrc(cmd);
	}
	else if (command == "PRIVMSG")
	{
		BString chat_id = params.First();
		BString user_id = _SenderIdent(sender);
		if (_IsChannelName(chat_id) == false)
			chat_id = _SenderNick(sender);

		BMessage chat(IM_MESSAGE);
		chat.AddInt32("im_what", IM_MESSAGE_RECEIVED);
		chat.AddString("chat_id", chat_id);
		chat.AddString("user_id", user_id);
		chat.AddString("user_name", _SenderNick(sender));
		chat.AddString("body", params.Last());
		_SendMsg(&chat);
	}
	else if (command == "NOTICE")
	{
		BString chat_id = params.First();
		BMessage send(IM_MESSAGE);
		send.AddInt32("im_what", IM_MESSAGE_RECEIVED);
		if (chat_id != "AUTH" && chat_id != "*") {
			send.AddString("chat_id", chat_id);
			sender = "";
		}
		if (sender.IsEmpty() == false)
			send.AddString("user_id", sender);
		send.AddString("body", params.Last());
		_SendMsg(&send);
	}
	else if (command == "TOPIC")
	{
		BString chat_id = params.First();
		BString subject = params.Last();

		BMessage topic(IM_MESSAGE);
		topic.AddInt32("im_what", IM_ROOM_SUBJECT_SET);
		topic.AddString("subject", subject);
		topic.AddString("chat_id", chat_id);
		_SendMsg(&topic);
	}
	else if (command == "JOIN")
	{
		BString chat_id = params.First();

		BMessage joined(IM_MESSAGE);
		joined.AddString("chat_id", chat_id);
		if (_SenderIdent(sender) == fIdent) {
			joined.AddInt32("im_what", IM_ROOM_JOINED);
			// Populate the userlist
			BString cmd("WHO ");
			cmd << chat_id << "\n";
			_SendIrc(cmd);

			fChannels.Add(chat_id);
		}
		else {
			joined.AddInt32("im_what", IM_ROOM_PARTICIPANT_JOINED);
			joined.AddString("user_id", _SenderIdent(sender));
			joined.AddString("user_name", _SenderNick(sender));
			fIdentNicks.AddItem(_SenderIdent(sender), _SenderNick(sender));
		}
		_SendMsg(&joined);
	}
	else if (command == "PART")
	{
		BString chat_id = params.First();
		BString body = B_TRANSLATE("left: ");
		body << params.Last();

		BMessage left(IM_MESSAGE);
		left.AddString("chat_id", chat_id);
		left.AddString("body", body);
		if (_SenderIdent(sender) == fIdent) {
			left.AddInt32("im_what", IM_ROOM_LEFT);
			fChannels.Remove(chat_id);
		}
		else {
			left.AddInt32("im_what", IM_ROOM_PARTICIPANT_LEFT);
			left.AddString("user_id", _SenderIdent(sender));
			left.AddString("user_name", _SenderNick(sender));
		}
		_SendMsg(&left);
	}
	else if (command == "QUIT")
	{
		BString body = B_TRANSLATE("quit: ");
		body << params.Last();

		for (int i = 0; i < fChannels.CountStrings(); i++) {
			BMessage left(IM_MESSAGE);
			left.AddInt32("im_what", IM_ROOM_PARTICIPANT_LEFT);
			left.AddString("user_id", _SenderIdent(sender));
			left.AddString("user_name", _SenderNick(sender));
			left.AddString("chat_id", fChannels.StringAt(i));
			_SendMsg(&left);
		}

		BMessage status(IM_MESSAGE);
		status.AddInt32("im_what", IM_USER_STATUS_SET);
		status.AddString("user_id", _SenderIdent(sender));
		status.AddInt32("status", STATUS_OFFLINE);
		_SendMsg(&status);
	}
	else if (command == "INVITE")
	{
		BMessage invite(IM_MESSAGE);
		invite.AddInt32("im_what", IM_ROOM_INVITE_RECEIVED);
		invite.AddString("chat_id", params.Last());
		invite.AddString("user_id", _SenderIdent(sender));
		_SendMsg(&invite);
	}
	else if (command == "NICK")
	{
		BString ident = _SenderIdent(sender);
		BString user_name = params.Last();

		BMessage nick(IM_MESSAGE);
		nick.AddString("user_name", user_name);
		if (ident == fIdent) {
			nick.AddInt32("im_what", IM_OWN_NICKNAME_SET);
			fNick = user_name;
		}
		else {
			nick.AddInt32("im_what", IM_USER_NICKNAME_SET);
			nick.AddString("user_id", ident);
			fIdentNicks.RemoveItemFor(ident);
			fIdentNicks.AddItem(ident, user_name);
		}
		_SendMsg(&nick);
	}
}


void
IrcProtocol::_MakeReady(BString nick, BString ident)
{
	fNick = nick;
	fIdent = ident;

	fReady = true;
	BMessage ready(IM_MESSAGE);
	ready.AddInt32("im_what", IM_PROTOCOL_READY);
	_SendMsg(&ready);

	BMessage self(IM_MESSAGE);
	self.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	self.AddString("user_id", fIdent);
	self.AddString("user_name", fNick);
	_SendMsg(&self);

	_SendIrc("MOTD\n");
}


BString
IrcProtocol::_LineSender(BStringList words)
{
	BString sender;
	if (words.CountStrings() > 1)
		sender = words.First().RemoveFirst(":");
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
IrcProtocol::_LineParameters(BStringList words, BString line)
{
	BStringList params;
	BString current;
	for (int i = 2; i < words.CountStrings(); i++)
		if ((current = words.StringAt(i)).StartsWith(":") == false)
			params.Add(current);
		else
			break;

	// Last parameter is preceded by a colon
	int32 index = line.RemoveChars(0, 1).FindFirst(" :");
	if (index != B_ERROR)
		params.Add(line.RemoveChars(0, index + 2));
	return params;
}


void
IrcProtocol::_SendMsg(BMessage* msg)
{
	msg->AddString("protocol", Signature());
	if (fReady == true)
		fMessenger->SendMessage(msg);
	else {
		std::cout << "Tried sending message when not ready: \n";
		msg->PrintToStream();
	}
}


void
IrcProtocol::_SendIrc(BString cmd)
{
	cmd << "\r\n";
	if (fSocket != NULL && fSocket->IsConnected() == true) {
		while (fWriteLocked == true)
			snooze(1000);
		fWriteLocked = true;
		fSocket->Write(cmd.String(), cmd.CountBytes(0, cmd.CountChars()));
		fWriteLocked = false;
	}
	else {
		BMessage disable(IM_MESSAGE);
		disable.AddInt32("im_what", IM_PROTOCOL_DISABLE);
	}
}


BString
IrcProtocol::_SenderNick(BString sender)
{
	BStringList split;
	sender.Split("!", true, split);
	return split.First();
}


BString
IrcProtocol::_SenderIdent(BString sender)
{
	BStringList split;
	sender.Split("!", true, split);
	return split.Last();
}


BString
IrcProtocol::_IdentNick(BString ident)
{
	bool found = false;
	BString nick = fIdentNicks.ValueFor(ident, &found);
	if (found == true)
		return nick;
	return ident;
}


bool
IrcProtocol::_IsChannelName(BString name)
{
	return (name.StartsWith("!") || name.StartsWith("&") || name.StartsWith("#")
		|| name.StartsWith("+"));
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
		total << buf;
		if (DEBUG_ENABLED)
			std::cerr << buf << std::endl;
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
	ssl.AddString("description", B_TRANSLATE("SSL"));
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
	ident.AddString("description", B_TRANSLATE("Username:"));
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
