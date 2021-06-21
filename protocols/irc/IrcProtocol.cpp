/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "IrcProtocol.h"

#include <cstdio>

#include <StringList.h>

#include <ChatProtocolMessages.h>


IrcProtocol* current_proto;


status_t
connect_thread(void* data)
{
	IrcProtocol* proto = (IrcProtocol*)data;
	current_proto = proto;

	BMessage* settings = proto->fSettings;
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

	// Now create the session
	irc_callbacks_t callbacks = get_callbacks();
	irc_session_t* session = irc_create_session(&callbacks);
	proto->fSession = session;

	irc_ctx_t ctx;
	ctx.nick.SetTo(nick);
	irc_set_ctx(session, &ctx);

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
	BString chat_id;
	if (msg->FindString("chat_id", &chat_id) != B_OK)
		return B_ERROR;

	switch (msg->FindInt32("im_what"))
	{
		case IM_SEND_MESSAGE:
		{
			BString body;
			if (msg->FindString("body", &body) != B_OK)
				return B_ERROR;
			irc_cmd_msg(fSession, chat_id.String(), body.String());

			irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(fSession);

			BMessage sendMsg(*msg);
			sendMsg.ReplaceInt32("im_what", IM_MESSAGE_SENT);
			sendMsg.AddString("user_id", ctx->id);
			_SendMessage(&sendMsg);
			break;
		}
		case IM_JOIN_ROOM:
		case IM_CREATE_ROOM:
		{
			irc_cmd_join(fSession, chat_id.String(), "");
			break;
		}
		case IM_LEAVE_ROOM:
		{
			irc_cmd_part(fSession, chat_id.String());
			break;
		}
		case IM_GET_ROOM_PARTICIPANTS:
		{
			irc_cmd_names(fSession, chat_id.String());
			break;
		}
		case IM_ROOM_SEND_INVITE:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				return B_ERROR;
			irc_cmd_invite(fSession, user_id.String(), chat_id.String());
			break;
		}
		case IM_ROOM_KICK_PARTICIPANT:
		{
			BString user_id;
			if (msg->FindString("user_id", &user_id) != B_OK)
				return B_ERROR;
			irc_cmd_kick(fSession, user_id.String(), chat_id.String(),
				msg->FindString("body"));
			break;
		}
	}
	return B_OK;
}


status_t
IrcProtocol::UpdateSettings(BMessage* msg)
{
	fSettings = msg;
	fServerThread = spawn_thread(connect_thread, "connect_thread",
		B_NORMAL_PRIORITY, (void*)this);

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
	callbacks.event_part = event_part;
	callbacks.event_channel = event_channel;
	callbacks.event_privmsg = event_privmsg;
	callbacks.event_nick = event_nick;
	return callbacks;
}


void
event_connect(irc_session_t* session, const char* event,
	const char* origin, const char** params, unsigned int count)
{
	BMessage readyMsg(IM_MESSAGE);
	readyMsg.AddInt32("im_what", IM_PROTOCOL_READY);
	_SendMessage(&readyMsg);
}


void
event_numeric(irc_session_t* session, unsigned int event,
	const char* origin, const char** params, unsigned int count)
{
	irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(session);

	switch (event) {
		case LIBIRC_RFC_RPL_NAMREPLY:
		{
			BStringList user_id;

			BString user_list(params[3]);
			user_list.Split(" ", false, user_id);

			BStringList user_name(user_id);
			int32 index = user_name.IndexOf(ctx->nick);
			if (index >= 0)
				user_id.Replace(index, BString(ctx->id));

			BMessage list(IM_MESSAGE);
			list.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
			list.AddString("chat_id", params[2]);
			list.AddStrings("user_id", user_id);
			list.AddStrings("user_name", user_name);
			_SendMessage(&list);
			break;
		}
		case LIBIRC_RFC_RPL_TOPIC:
		{
			BMessage topic(IM_MESSAGE);
			topic.AddInt32("im_what", IM_ROOM_SUBJECT_SET);
			topic.AddString("chat_id", params[1]);
			topic.AddString("subject", params[2]);
			_SendMessage(&topic);
			break;
		}
	}
}


void
event_join(irc_session_t* session, const char* event, const char* joiner,
	const char** channel, unsigned int count)
{
	int32 im_what = IM_ROOM_PARTICIPANT_JOINED;

	irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(session);
	if (_IsOwnUser(joiner, session) == true)
		im_what = IM_ROOM_JOINED;

	BMessage joinedMsg(IM_MESSAGE);
	joinedMsg.AddInt32("im_what", im_what);
	joinedMsg.AddString("user_id", joiner);
	joinedMsg.AddString("chat_id", channel[0]);
	_SendMessage(&joinedMsg);
}


void
event_part(irc_session_t* session, const char* event, const char* quitter,
	const char** chanReason, unsigned int count)
{
	int32 im_what = IM_ROOM_PARTICIPANT_LEFT;
	if (_IsOwnUser(quitter, session) == true)
		im_what = IM_ROOM_LEFT;

	BString body;
	if (chanReason[1] != NULL)
		body << chanReason[1];

	BMessage quitMsg(IM_MESSAGE);
	quitMsg.AddInt32("im_what", im_what);
	quitMsg.AddString("user_id", quitter);
	quitMsg.AddString("chat_id", chanReason[0]);
	if (body.IsEmpty() == false)
		quitMsg.AddString("body", body);
	_SendMessage(&quitMsg);
}


void
event_channel(irc_session_t* session, const char* event, const char* sender,
	const char** chanBody, unsigned int count)
{
	BMessage msgMsg(IM_MESSAGE);
	msgMsg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msgMsg.AddString("user_id", sender);
	msgMsg.AddString("chat_id", chanBody[0]);
	msgMsg.AddString("body", chanBody[1]);
	_SendMessage(&msgMsg);
}


void
event_privmsg(irc_session_t* session, const char* event, const char* sender,
	const char** selfBody, unsigned int count)
{
	BMessage msgMsg(IM_MESSAGE);
	msgMsg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msgMsg.AddString("user_id", sender);
	msgMsg.AddString("chat_id", _UserNick(sender));
	msgMsg.AddString("body", selfBody[1]);
	_SendMessage(&msgMsg);
}


// TODO: Nick-change API
void
event_nick(irc_session_t* session, const char* event, const char* oldNick,
	const char** newNick, unsigned int count)
{
	irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(session);

	if (_IsOwnUser(oldNick, session) == true) {
		ctx->nick.SetTo(newNick[0]);
		irc_set_ctx(session, ctx);
	}
}


BString
_UserNick(const char* userId)
{
	BStringList split;
	BString id(userId);
	id.Split("!", false, split);

	BString name = split.StringAt(0);
	if (name.StartsWith("@") == true)
		name.RemoveFirst("@");

	return name;
}


bool
_IsOwnUser(const char* userId, irc_session_t* session)
{
	bool ret = false;
	irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(session);

	if (ctx->nick == _UserNick(userId)) {
		ret = true;
		if (ctx->id.IsEmpty() == true) {
			ctx->id.SetTo(userId);
			irc_set_ctx(session, ctx);
			
			// The app hasn't been informed of user_id, then!
			BMessage ownInfo(IM_MESSAGE);
			ownInfo.AddInt32("im_what", IM_OWN_CONTACT_INFO);
			ownInfo.AddString("user_id", userId);
			_SendMessage(&ownInfo);
		}
	}
	return ret;
}


void
_SendMessage(BMessage* msg)
{
	if (!msg)
		return;
	msg->AddString("protocol", current_proto->Signature());
	current_proto->MessengerInterface()->SendMessage(msg);
}
