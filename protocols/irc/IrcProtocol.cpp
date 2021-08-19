/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * Copyright 2017, Akshay Agarwal <agarwal.akshay.akshay8@gmail.com>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IrcProtocol.h"

#include <iostream>

#include <Catalog.h>
#include <Directory.h>
#include <FindDirectory.h>
#include <Font.h>
#include <Resources.h>
#include <SecureSocket.h>
#include <Socket.h>

#include <libinterface/BitmapUtils.h>

#include <AppConstants.h>
#include <Cardie.h>
#include <ChatProtocolMessages.h>
#include <Flags.h>
#include <Utils.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "IrcProtocol"


const int32 IRC_CMD = 'ICmd';


status_t
connect_thread(void* data)
{
	IrcProtocol* protocol = (IrcProtocol*)data;
	protocol->Connect();
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
	_SaveContacts();

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
	fRealName = settings->FindString("real_name");
	fServer = settings->FindString("server");
	fPassword = settings->FindString("password");
	fPort = settings->FindInt32("port");
	fSsl = settings->GetBool("ssl", false);

	fRecvThread = spawn_thread(connect_thread, "what_a_tangled_web_we_weave",
		B_NORMAL_PRIORITY, (void*)this);

	if (fRecvThread < B_OK)
		return B_ERROR;
	return B_OK;
}


status_t
IrcProtocol::Process(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");
	switch (im_what) {
		case IRC_CMD:
		{
			BStringList words;
			BString line = msg->GetString("misc_str", "");
			line.Split(" ", true, words);

			BString command = words.First();
			if (command.ICompare("WHOIS") == 0)
				fWhoIsRequested = true;
			else if (command.ICompare("WHO") == 0)
				fWhoRequested = true;

			_SendIrc(line);
			break;
		}
		case IM_SET_OWN_STATUS:
		{
			int32 status = msg->FindInt32("status");
			BString status_msg = msg->FindString("message");

			BMessage statusSet(IM_MESSAGE);
			statusSet.AddInt32("im_what", IM_OWN_STATUS_SET);

			switch (status) {
				case STATUS_ONLINE:
					statusSet.AddInt32("status", STATUS_ONLINE);
					resume_thread(fRecvThread);
					break;
				case STATUS_OFFLINE:
					statusSet.AddInt32("status", STATUS_OFFLINE);
					Shutdown();
					break;
				default:
					break;
			}
			if (status == STATUS_ONLINE || status == STATUS_OFFLINE)
				_SendMsg(&statusSet);
			break;
		}
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
		case IM_CREATE_CHAT:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;
			BString user_name = _IdentNick(user_id);

			if (user_name != user_id) {
				BMessage created(IM_MESSAGE);
				created.AddInt32("im_what", IM_CHAT_CREATED);
				created.AddString("chat_id", user_name);
				created.AddString("user_id", user_id);
				_SendMsg(&created);
				fChannels.Add(user_name);
				break;
			}
			// If it's not a known user, we need to get their ID/nick somehow
			// … that is, through the WHO.
			fWhoIm = user_id;
			BString cmd("WHOIS ");
			cmd << user_id << "\n";
			_SendIrc(cmd);
			break;

		}
		case IM_JOIN_ROOM:
		case IM_CREATE_ROOM:
		case IM_ROOM_INVITE_ACCEPT:
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
					fChannels.Remove(chat_id);
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
		case IM_GET_ROOM_PARTICIPANTS:
		{
			BString chat_id;
			if (msg->FindString("chat_id", &chat_id) != B_OK)
				break;

			// Rooms are populated with RPL_WHOREPLY, chats RPL_WHOISUSER
			BString cmd;
			if (_IsChannelName(chat_id) == true)
				cmd = "WHO ";
			else
				cmd = "WHOIS ";
			cmd << chat_id << "\n";
			_SendIrc(cmd);
			break;
		}
		case IM_ROOM_BAN_PARTICIPANT:
		case IM_ROOM_KICK_PARTICIPANT:
		{
			BString chat_id = msg->FindString("chat_id");
			BString user_id = msg->FindString("user_id");
			BString body;

			if (chat_id.IsEmpty() == true || user_id.IsEmpty() == true)
				break;

			if (im_what == IM_ROOM_BAN_PARTICIPANT) {
				BString cmd("MODE ");
				cmd << chat_id << " +b " << _IdentNick(user_id) << "!*@*";
				_SendIrc(cmd);
			}

			if (msg->FindString("body", &body) != B_OK
					&& im_what == IM_ROOM_BAN_PARTICIPANT)
				body = B_TRANSLATE("You've been banned, nerd");
			else if (body.IsEmpty() == true)
				body = B_TRANSLATE("Watch the door on your way out");

			BString cmd("KICK ");
			cmd << chat_id << " " << _IdentNick(user_id);
			cmd << " :" << body;
			_SendIrc(cmd);
			break;
		}
		case IM_ROOM_UNBAN_PARTICIPANT:
		{
			BString chat_id = msg->FindString("chat_id");
			BString user_id = msg->FindString("user_id");

			if (chat_id.IsEmpty() == false && user_id.IsEmpty() == false) {
				BString cmd("MODE ");
				cmd << chat_id << " -b " << _IdentNick(user_id) << "!*@*";
				_SendIrc(cmd);
			}
			break;
		}
		case IM_ROOM_SEND_INVITE:
		{
			BString chat_id = msg->FindString("chat_id");
			BString user_id = msg->FindString("user_id");
			if (chat_id.IsEmpty() == false && user_id.IsEmpty() == false) {
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
		case IM_ROSTER_ADD_CONTACT:
		{
			BString user_nick;
			if (msg->FindString("user_id", &user_nick) == B_OK)
				_AddContact(user_nick);
			break;
		}
		case IM_ROSTER_REMOVE_CONTACT:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) == B_OK)
				_RemoveContact(_NickIdent(user_id));
			break;
		}
		case IM_GET_CONTACT_INFO:
		case IM_GET_EXTENDED_CONTACT_INFO:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				break;

			BMessage info(IM_MESSAGE);
			if (im_what == IM_GET_CONTACT_INFO)
				info.AddInt32("im_what", IM_CONTACT_INFO);
			else
				info.AddInt32("im_what", IM_EXTENDED_CONTACT_INFO);
			info.AddString("user_id", user_id);
			info.AddString("user_name", _IdentNick(user_id));
			if (fOfflineContacts.HasString(_IdentNick(user_id)) == true)
				info.AddInt32("status", (int32)STATUS_OFFLINE);
			else
				info.AddInt32("status", (int32)STATUS_ONLINE);
			_SendMsg(&info);
			break;
		}
		case IM_SET_ROOM_SUBJECT:
		{
			BString chat_id;
			BString body = msg->FindString("subject");
			if (msg->FindString("chat_id", &chat_id) == B_OK) {
				BString cmd("TOPIC ");
				cmd << chat_id << " :" << body;
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
	else if (strcmp(name, "roster") == 0)
		settings = _RosterTemplate();
	return settings;
}


BObjectList<BMessage>
IrcProtocol::Commands()
{
	BMessage handle(IM_MESSAGE);
	handle.AddInt32("im_what", IRC_CMD);
	handle.AddString("cmd_name", "/");

	BMessage* cmd = new BMessage();
	cmd->AddString("_name", "/");
	cmd->AddString("_desc", B_TRANSLATE("Send a raw IRC command to the server. E.g., '// HELP' or '// WHOIS'."));
	cmd->AddBool("_proto", true);
	cmd->AddMessage("_msg", &handle);
	cmd->AddString("class", "ChatCommand");

	BObjectList<BMessage> cmds;
	cmds.AddItem(cmd);
	return cmds;
}


BBitmap*
IrcProtocol::Icon() const
{
	return ReadNodeIcon(fAddOnPath.Path(), B_LARGE_ICON, true);
}


status_t
IrcProtocol::Connect()
{
	fSocket = fSsl ? new BSecureSocket : new BSocket;

	if (fSocket->Connect(BNetworkAddress(fServer, fPort)) != B_OK)
		return B_ERROR;

	if (fPassword.IsEmpty() == false) {
		BString passMsg = "PASS ";
		passMsg << fPassword;
		_SendIrc(passMsg);
	}

	BString userMsg = "USER ";
	userMsg << fUser << " * 0 :" << fRealName;
	_SendIrc(userMsg);

	BString nickMsg = "NICK ";
	nickMsg << fNick;
	_SendIrc(nickMsg);
	return B_OK;
}


status_t
IrcProtocol::Loop()
{
	fWhoIsRequested = false;
	fWhoRequested = false;
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

	// Act on the numeric as appropriate
	switch (numeric) {
		case RPL_WELCOME:
		{
			if (params.CountStrings() == 2)
				fNick = params.First();
			BString cmd("WHOIS ");
			cmd << fNick << "\n";
			_SendIrc(cmd);
			break;
		}
		case RPL_WHOISUSER:
		{
			BString nick = params.StringAt(1);
			BString user = params.StringAt(2);
			BString host = params.StringAt(3);
			BString ident = user;
			ident << "@" << host;

			fIdentNicks.RemoveItemFor(ident);
			fIdentNicks.AddItem(ident, nick);

			// If is a contact, let's go!
			_UpdateContact(nick, ident, true);

			// Contains the own user's contact info― protocol ready!
			if (fReady == false && nick == fNick) {
				fUser = user.String();
				_MakeReady(nick, ident);
			}
			// Used in the creation of a one-on-one chat
			else if (fWhoIm == user || fWhoIm == nick) {
				fWhoIm = "";
				BMessage created(IM_MESSAGE);
				created.AddInt32("im_what", IM_CHAT_CREATED);
				created.AddString("chat_id", nick);
				created.AddString("user_id", ident);
				_SendMsg(&created);
				fChannels.Add(nick);
			}
			// Used to populate a one-on-one chat's userlist… lol, I know.
			else if (fWhoIsRequested == false && fChannels.HasString(nick)) {
				BMessage user(IM_MESSAGE);
				user.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
				user.AddString("chat_id", nick);
				user.AddString("user_id", ident);
				user.AddString("user_name", nick);
				_SendMsg(&user);
			}
			break;
		}
		case RPL_WHOREPLY:
		{
			BString channel = params.StringAt(1);
			BString user = params.StringAt(2);
			BString host = params.StringAt(3);
			BString nick = params.StringAt(5);
			BString role = params.StringAt(6);
			BString ident = user;
			ident << "@" << host;

			fIdentNicks.RemoveItemFor(ident);
			fIdentNicks.AddItem(ident, nick);

			// Used to populate a room's userlist (one-by-one… :p)
			if (fWhoRequested == false && _IsChannelName(channel)) {
				// Send the participant themself
				BMessage user(IM_MESSAGE);
				user.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
				user.AddString("chat_id", channel);
				user.AddString("user_id", ident);
				user.AddString("user_name", nick);
				_SendMsg(&user);

				// Now let's crunch the appropriate role…
				bool away = false;
				UserRole priority = ROOM_MEMBER;
				for (int i=0; i < role.CountBytes(0, role.CountChars()); i++) {
					char c = role.ByteAt(i);
					switch (c) {
						case 'G':
						case 'H':
							away = false;
							break;
						case 'A':
							away = true;
							break;
						case '%':
							priority = ROOM_HALFOP;
							break;
						case '@':
							priority = ROOM_OPERATOR;
							break;
						case '*':
							priority = IRC_OPERATOR;
							break;
					}
				}

				// And send the user's role
				BMessage sensei(IM_MESSAGE);
				sensei.AddInt32("im_what", IM_ROOM_ROLECHANGED);
				sensei.AddString("chat_id", channel);
				sensei.AddString("user_id", ident);
				sensei.AddInt32("role_priority", priority);
				sensei.AddString("role_title", _RoleTitle(priority));
				sensei.AddInt32("role_perms", _RolePerms(priority));
				_SendMsg(&sensei);

				// Also status! Can't forget that
				BMessage status(IM_MESSAGE);
				status.AddInt32("im_what", IM_USER_STATUS_SET);
				status.AddString("user_id", ident);
				if (away == true)
					status.AddInt32("status", STATUS_AWAY);
				else
					status.AddInt32("status", STATUS_ONLINE);
				_SendMsg(&status);
			}
			break;
		}
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
	}

	// Now, to determine if the line should be sent to system buffer
	switch (numeric) {
		case RPL_ENDOFWHO:
			fWhoRequested = false;
			break;
		case RPL_ENDOFWHOIS:
			fWhoIsRequested = false;
			break;
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
		case RPL_WHOREPLY:
			if (fWhoRequested == false)
				break;
		case RPL_WHOISUSER:
			if (fWhoIsRequested == false)
				break;
		default:
			BMessage send(IM_MESSAGE);
			send.AddInt32("im_what", IM_MESSAGE_RECEIVED);
			send.AddString("body", line);
			_SendMsg(&send);
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
		BString user_name = _SenderNick(sender);
		BString body = params.Last();
		if (_IsChannelName(chat_id) == false)
			chat_id = _SenderNick(sender);
		if (fChannels.HasString(chat_id) == false)
			fChannels.Add(chat_id);

		_UpdateContact(user_name, user_id, true);

		BMessage chat(IM_MESSAGE);
		chat.AddInt32("im_what", IM_MESSAGE_RECEIVED);
		chat.AddString("chat_id", chat_id);
		chat.AddString("user_id", user_id);
		chat.AddString("user_name", user_name);
		_AddFormatted(&chat, "body", body);
		_SendMsg(&chat);
	}
	else if (command == "NOTICE")
	{
		BString chat_id = params.First();
		BMessage send(IM_MESSAGE);
		send.AddInt32("im_what", IM_MESSAGE_RECEIVED);

		if (_IsChannelName(chat_id) == false)
			chat_id = _SenderNick(sender);
		if (fChannels.HasString(chat_id) == false)
			fChannels.Add(chat_id);

		if (chat_id != "AUTH" || chat_id != "*")
			send.AddString("chat_id", chat_id);

		if (sender.IsEmpty() == false) {
			send.AddString("user_id", _SenderIdent(sender));
			send.AddString("user_name", _SenderNick(sender));
		}
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
		BString user_id = _SenderIdent(sender);
		BString user_name = _SenderNick(sender);
		_UpdateContact(user_name, user_id, true);

		BMessage joined(IM_MESSAGE);
		joined.AddString("chat_id", chat_id);
		if (_SenderIdent(sender) == fIdent) {
			joined.AddInt32("im_what", IM_ROOM_JOINED);
			fChannels.Add(chat_id);
		}
		else {
			joined.AddInt32("im_what", IM_ROOM_PARTICIPANT_JOINED);
			joined.AddString("user_id", user_id);
			joined.AddString("user_name", user_name);
			fIdentNicks.AddItem(user_id, user_name);
		}
		_SendMsg(&joined);

		BMessage status(IM_MESSAGE);
		status.AddInt32("im_what", IM_USER_STATUS_SET);
		status.AddString("user_id", user_id);
		status.AddInt32("status", STATUS_ONLINE);
		_SendMsg(&status);
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
	else if (command == "KICK")
	{
		BString chat_id = params.First();
		BString user_id = params.StringAt(1);

		BMessage foot(IM_MESSAGE);
		foot.AddInt32("im_what", IM_ROOM_PARTICIPANT_KICKED);
		foot.AddString("chat_id", chat_id);
		foot.AddString("user_name", _IdentNick(user_id));
		foot.AddString("user_id", _NickIdent(user_id));
		if (params.CountStrings() == 3)
			foot.AddString("body", params.StringAt(2));
		_SendMsg(&foot);
	}
	else if (command == "QUIT")
	{
		BString user_id = _SenderIdent(sender);
		BString user_name = _SenderNick(sender);
		_UpdateContact(user_name, user_id, false);

		BString body = B_TRANSLATE("quit: ");
		body << params.Last();

		for (int i = 0; i < fChannels.CountStrings(); i++) {
			if (_IsChannelName(fChannels.StringAt(i)) == false)
				continue;
			BMessage left(IM_MESSAGE);
			left.AddInt32("im_what", IM_ROOM_PARTICIPANT_LEFT);
			left.AddString("user_id", user_id);
			left.AddString("user_name", user_name);
			left.AddString("chat_id", fChannels.StringAt(i));
			_SendMsg(&left);
		}

		BMessage status(IM_MESSAGE);
		status.AddInt32("im_what", IM_USER_STATUS_SET);
		status.AddString("user_id", user_id);
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

			_RenameContact(ident, user_name);
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

	_LoadContacts();
	_JoinDefaultRooms();
}


BString
IrcProtocol::_LineSender(BStringList words)
{
	BString sender;
	if (words.CountStrings() > 1) {
		sender = words.First();
		if (sender.StartsWith(":") == true)
			sender.RemoveFirst(":");
		else if (sender.StartsWith("*:") == true)
			sender.RemoveFirst("*:");
	}
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
	else if (DEBUG_ENABLED == true) {
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


BString
IrcProtocol::_NickIdent(BString nick)
{
	for (int i = 0; i < fIdentNicks.CountItems(); i++)
		if (fIdentNicks.ValueAt(i) == nick)
			return fIdentNicks.KeyAt(i);
	return nick;
}


bool
IrcProtocol::_IsChannelName(BString name)
{
	return (name.StartsWith("!") || name.StartsWith("&") || name.StartsWith("#")
		|| name.StartsWith("+"));
}


#define disable_all_faces(current) { \
	if (bold > -1)		_FormatToggleFace(msg, B_BOLD_FACE, &bold, current); \
	if (italics > -1)	_FormatToggleFace(msg, B_ITALIC_FACE, &italics, current); \
	if (strike > -1)	_FormatToggleFace(msg, B_STRIKEOUT_FACE, &strike, current); \
	if (underline > -1)	_FormatToggleFace(msg, B_UNDERSCORE_FACE, &underline, current); \
	if (reverse > -1)	_FormatToggleFace(msg, B_NEGATIVE_FACE, &reverse, current); \
};


#define disable_color(current) { \
	if (colorStart > -1) { \
		msg->AddInt32("color_start", colorStart); \
		msg->AddInt32("color_length", current - colorStart); \
		msg->AddColor("color", _IntToRgb(color)); \
		colorStart = -1; \
		color = -1; \
	} \
};


void
IrcProtocol::_AddFormatted(BMessage* msg, const char* name, BString text)
{
	BString newText;
	int32 italics = -1, bold = -1, underline = -1, strike = -1, mono = -1;
	int32 reverse = -1;
	int32 color = -1, colorStart = -1;

	int32 length = text.CountBytes(0, text.CountChars());
	for (int32 j=0, i=0; j < length; j++) {
		char c = text.ByteAt(j);

		switch (c) {
			case FORMAT_COLOR:
			{
				// Apply and reset previous color, if existant
				disable_color(i);

				char one = text.ByteAt(j + 1);
				char two = text.ByteAt(j + 2);

				// Try and get colors from either two-digits or one-digits
				int32 colorIndex = -1;
				if (isdigit(one) == true && isdigit(two) == true) {
					char num[3] = { one, two, '\0' };
					colorIndex = atoi(num);
					if (colorIndex >= 0 && colorIndex <= 99)
						j += 2;
				}
				else if (isdigit(one) == true) {
					char num[2] = { one, '\0' };
					colorIndex = atoi(num);
					if (colorIndex >= 0 && colorIndex <= 99)
						j++;
				}
				else
					break;

				// Use color if valid
				if (colorIndex >= 0 && colorIndex <= 99) {
					int colors[FORMAT_COLOR_COUNT] = FORMAT_COLORS;
					color = colors[colorIndex];
					colorStart = i;
				}

				// Ignore setting of background
				if (text.ByteAt(j + 1) == ',')
					if (isdigit(text.ByteAt(j+2)) && isdigit(text.ByteAt(j+3)))
						j += 3;
					else if (isdigit(text.ByteAt(j + 2)))
						j += 2;
				break;
			}
			case FORMAT_BOLD:
				_FormatToggleFace(msg, B_BOLD_FACE, &bold, i);
				break;
			case FORMAT_ITALIC:
				_FormatToggleFace(msg, B_ITALIC_FACE, &italics, i);
				break;
			case FORMAT_UNDERSCORE:
				_FormatToggleFace(msg, B_UNDERSCORE_FACE, &underline, i);
				break;
			case FORMAT_STRIKEOUT:
				_FormatToggleFace(msg, B_STRIKEOUT_FACE, &strike, i);
				break;
			case FORMAT_REVERSE:
				_FormatToggleFace(msg, B_NEGATIVE_FACE, &reverse, i);
				break;
			case FORMAT_RESET:
				disable_all_faces(i);
				disable_color(i);
				break;
			default:
				newText << c;
				i++;
		}
	}
	disable_all_faces(length);
	disable_color(length);
	msg->AddString(name, newText);
}


void
IrcProtocol::_FormatToggleFace(BMessage* msg, uint16 face, int32* start,
	int32 current)
{
	if (*start == -1)
		*start = current;
	else {
		msg->AddInt32("face_start", *start);
		msg->AddInt32("face_length", current - *start);
		msg->AddUInt16("face", face);
		*start = -1;
	}
}


void
IrcProtocol::_UpdateContact(BString nick, BString ident, bool online)
{
	if (fContacts.HasString(nick) == false)
		return;

	if (online == true && fOfflineContacts.HasString(nick) == true) {
		_RemoveContact(nick);
		_AddContact(ident);
		fOfflineContacts.Remove(nick);
	}
	else if (online == false && fOfflineContacts.HasString(nick) == false) {
		fOfflineContacts.Add(nick);
	}
}


void
IrcProtocol::_AddContact(BString nick)
{
	fContacts.Add(nick);
	BString user_id = _NickIdent(nick);

	BMessage added(IM_MESSAGE);
	added.AddInt32("im_what", IM_ROSTER);
	added.AddString("user_id", user_id);
	_SendMsg(&added);

	if (user_id == nick)
		fOfflineContacts.Add(nick);
}


void
IrcProtocol::_RemoveContact(BString user_id)
{
	BString nick = _IdentNick(user_id);
	fContacts.Remove(nick);
	fOfflineContacts.Remove(nick);

	BMessage removed(IM_MESSAGE);
	removed.AddInt32("im_what", IM_ROSTER_CONTACT_REMOVED);
	removed.AddString("user_id", _NickIdent(user_id));
	_SendMsg(&removed);
}


void
IrcProtocol::_RenameContact(BString user_id, BString newNick)
{
	BString oldNick = _IdentNick(user_id);

	if (fContacts.HasString(oldNick) == true) {
		fContacts.Remove(_IdentNick(user_id));
		fOfflineContacts.Remove(_IdentNick(user_id));
		fContacts.Add(newNick);
	}
}


void
IrcProtocol::_LoadContacts()
{
	BMessage contacts;
	BFile file(_ContactsCache(), B_READ_ONLY);
	if (file.InitCheck() == B_OK)
		contacts.Unflatten(&file);

	int i = 0;
	BString user_name;
	while (contacts.FindString("user_name", i, &user_name) == B_OK) {
		_AddContact(user_name);
		i++;
	}
}


void
IrcProtocol::_SaveContacts()
{
	BMessage contacts;
	for (int i = 0; i < fContacts.CountStrings(); i++)
		contacts.AddString("user_name", fContacts.StringAt(i));

	BFile file(_ContactsCache(), B_WRITE_ONLY | B_CREATE_FILE);
	if (file.InitCheck() == B_OK)
		contacts.Flatten(&file);
}


int32
IrcProtocol::_RolePerms(UserRole role)
{
	switch (role) {
		case ROOM_HALFOP:
			return PERM_READ | PERM_WRITE | PERM_NICK | PERM_KICK
				| PERM_ROOM_SUBJECT;
		case IRC_OPERATOR:
		case ROOM_OPERATOR:
			return PERM_READ | PERM_WRITE | PERM_NICK | PERM_KICK | PERM_BAN
				| PERM_ROOM_SUBJECT | PERM_ROLECHANGE;
	}
	return PERM_READ | PERM_WRITE | PERM_NICK;
}


const char*
IrcProtocol::_RoleTitle(UserRole role)
{
	switch (role) {
		case ROOM_HALFOP:
			return B_TRANSLATE("Half-Op");
		case ROOM_OPERATOR:
			return B_TRANSLATE("Operator");
		case IRC_OPERATOR:
			return B_TRANSLATE("IRC Wizard");
	}
	return B_TRANSLATE("Member");
}


const char*
IrcProtocol::_ContactsCache()
{
	BPath path(AccountCachePath(fName));
	path.Append("contact_list");
	return path.Path();
}


void
IrcProtocol::_JoinDefaultRooms()
{
	// Hardcoded default room… I'm so awful, aren't I? ;-)
	if (fServer == "irc.oftc.net") {
		BFile room(RoomCachePath(fName, "#haiku"), B_READ_ONLY);
		if (room.InitCheck() != B_OK) {
			BString cmd("JOIN #haiku");
			_SendIrc(cmd);
		}
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


rgb_color
IrcProtocol::_IntToRgb(int rgb)
{
	uint8 r = rgb >> 16;
	uint8 g = rgb >> 8 & 0xFF;
	uint8 b = rgb & 0xFF;
	return (rgb_color){r, g, b};
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
	realName.AddString("default", "Gord the Destroyer");
	realName.AddString("error", B_TRANSLATE("A real name must be defined. (P.S.: You can lie!)"));
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


BMessage
IrcProtocol::_RosterTemplate()
{
	BMessage settings;

	BMessage nick;
	nick.AddString("name", "user_id");
	nick.AddString("description", B_TRANSLATE("Nick:"));
	nick.AddString("error", B_TRANSLATE("How can someone be your friend if you don't know their name?"));
	nick.AddInt32("type", B_STRING_TYPE);
	settings.AddMessage("setting", &nick);

	return settings;
}
