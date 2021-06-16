/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CHAT_COMMAND_H
#define CHAT_COMMAND_H

#include <Archivable.h>
#include <Message.h>
#include <String.h>

#include <libsupport/KeyMap.h>

class Conversation;


enum cmd_arg_type
{
	CMD_ROOM_PARTICIPANT	= 'CArp',	// "user_id" in message
	CMD_KNOWN_USER			= 'CAku',	// "user_id" in message
	CMD_ANY_USER			= 'CAau',	// "user_id" in message
	CMD_BODY_STRING			= 'CAbs',	// "body" in message
	CMD_MISC_STRING			= 'CAms'	// "misc_str" in message
};


class ChatCommand : public BArchivable {
public:
					ChatCommand(const char* name, BMessage msg, bool toProtocol,
								List<int32> argTypes);
					ChatCommand(BMessage* data);

	status_t		Archive(BMessage* data, bool deep=true);
	ChatCommand*	Instantiate(BMessage* data);


	const char*		GetName() { return fName.String(); }

	void			SetDesc(const char* description);
	const char*		GetDesc() { return fDescription.String(); }

	bool			Parse(BString args, BString* errorMsg, Conversation* chat);

private:
	bool			_ProcessArgs(BString args, BMessage* msg, BString* errorMsg,
								 Conversation* chat);

	bool			_Send(BMessage* msg, Conversation* chat);

	BString fName;
	BString fDescription;
	BMessage fMessage;

	bool fToProto;
	List<int32> fArgTypes;
};


typedef KeyMap<BString, ChatCommand*> CommandMap;


#endif // CHAT_COMMAND_H

