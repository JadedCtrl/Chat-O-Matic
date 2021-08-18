/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ChatCommand.h"

#include <Catalog.h>
#include <StringList.h>

#include "Conversation.h"
#include "MainWindow.h"
#include "TheApp.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ChatCommand"


ChatCommand::ChatCommand(const char* name, BMessage msg, bool toProtocol,
						 List<int32> argTypes)
	:
	fName(name),
	fMessage(msg),
	fToProto(toProtocol),
	fArgTypes(argTypes)
{
}


ChatCommand::ChatCommand(BMessage* data)
	: BArchivable(data)
{
	data->FindString("_name", &fName);
	data->FindString("_desc", &fDescription);
	data->FindBool("_proto", &fToProto);
	data->FindMessage("_msg", &fMessage);

	int32 argType, i = 0;
	while (data->FindInt32("_argtype", i, &argType) == B_OK) {
		fArgTypes.AddItem(argType);
		i++;
	}
}


status_t
ChatCommand::Archive(BMessage* data, bool deep)
{
	status_t ret = BArchivable::Archive(data, deep);
	data->AddString("_name", fName);
	data->AddString("_desc", fDescription);
	data->AddBool("_proto", fToProto);
	data->AddMessage("_msg", new BMessage(fMessage));

	for (int i = 0; i < fArgTypes.CountItems(); i++)
		data->AddInt32("_argtype", fArgTypes.ItemAt(i));
	return ret;
}


ChatCommand*
ChatCommand::Instantiate(BMessage* data)
{
	if (!validate_instantiation(data, "ChatCommand"))
		return NULL;
	return new ChatCommand(data);
}


void
ChatCommand::SetDesc(const char* description)
{
	fDescription = description;
}


bool
ChatCommand::Parse(BString args, BString* errorMsg, Conversation* chat)
{
	BMessage* msg = new BMessage(fMessage);
	msg->AddString("chat_id", chat->GetId());
	msg->AddInt64("instance", chat->GetProtocolLooper()->GetInstance());

	if (fArgTypes.CountItems() == 0) {
		msg->AddString("misc_str", args);
		return _Send(msg, chat);
	}

	if (_ProcessArgs(args, msg, errorMsg, chat) == true)
		return _Send(msg, chat);
	return false;
}


bool
ChatCommand::_ProcessArgs(BString args, BMessage* msg, BString* errorMsg,
					  Conversation* chat)
{
	int32 argCount = fArgTypes.CountItems();
	BStringList argList;
	args.Split(" ", false, argList);

	for (int i = 0; i < argCount; i++) {
		BString arg = argList.StringAt(i);
		const char* strName = "misc_str";

		switch (fArgTypes.ItemAt(i))
		{
			case CMD_ROOM_PARTICIPANT:
			{
				User* user = _FindUser(arg, chat->Users());
				if (user == NULL) {
					errorMsg->SetTo(B_TRANSLATE("%user% isn't a member of this "
						"room."));
					errorMsg->ReplaceAll("%user%", arg);
					return false;
				}
				msg->AddString("user_id", user->GetId());
				break;
			}
			case CMD_KNOWN_USER:
			{
				User* user = _FindUser(arg, chat->GetProtocolLooper()->Users());
				if (user == NULL) {
					errorMsg->SetTo(B_TRANSLATE("You aren't contacts with and "
						"have no chats in common with %user%. Shame."));
					errorMsg->ReplaceAll("%user%", arg);
					return false;
				}
				msg->AddString("user_id", user->GetId());
				break;
			}
			case CMD_ANY_USER:
				msg->AddString("user_id", arg);
				break;
			case CMD_BODY_STRING:
				strName = "body";
			default:
				// If string's the last argument, it can be longer than one word
				if (i == (argCount - 1) && argList.CountStrings() > argCount)
					for (int j = i + 1; j < argList.CountStrings(); j++)
						arg << " " << argList.StringAt(j);
				msg->AddString(strName, arg);
		}
	}
	return true;
}


User*
ChatCommand::_FindUser(BString idOrName, UserMap users)
{
	if (idOrName.IsEmpty() == true)
		return NULL;

	bool idFound = false;
	User* user = users.ValueFor(idOrName, &idFound);
	if (idFound == false)
		for (int i = 0; i < users.CountItems(); i++) {
			User* check = users.ValueAt(i);
			if (check != NULL && check->GetName() == idOrName)
				return check;
		}
	return user;
}


bool
ChatCommand::_Send(BMessage* msg, Conversation* chat)
{
	if (fToProto == true)
		chat->GetProtocolLooper()->PostMessage(msg);
	else
		((TheApp*)be_app)->GetMainWindow()->PostMessage(msg);
	return true;
}


