/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "DefaultItems.h"

#include <Looper.h>
#include <InterfaceDefs.h>

#include "AppMessages.h"
#include "ChatProtocolMessages.h"
#include "ChatCommand.h"
#include "Role.h"


BObjectList<BMessage>
DefaultCommands()
{
	List<int32> roomUser;
	roomUser.AddItem(CMD_ROOM_PARTICIPANT);
	List<int32> kickBody;
	kickBody.AddItem(CMD_ROOM_PARTICIPANT);
	kickBody.AddItem(CMD_BODY_STRING);
	List<int32> knownUser;
	knownUser.AddItem(CMD_KNOWN_USER);
	List<int32> anyUser;
	anyUser.AddItem(CMD_ANY_USER);

	CommandMap commands;

	BMessage kickMsg(IM_MESSAGE);
	kickMsg.AddInt32("im_what", IM_ROOM_KICK_PARTICIPANT);
	ChatCommand* kick = new ChatCommand("kick", kickMsg, true, kickBody);
	kick->SetDesc("Force a user to temporarily leave the room, assuming your "
				  "power level's high enough.");
	commands.AddItem("kick", kick);

	BMessage banMsg(IM_MESSAGE);
	banMsg.AddInt32("im_what", IM_ROOM_BAN_PARTICIPANT);
	ChatCommand* ban = new ChatCommand("ban", banMsg, true, kickBody);
	ban->SetDesc("Kick a user out of the room and slam the door behind them― "
				 "locking it while you're at it.");
	commands.AddItem("ban", ban);

	BMessage unbanMsg(IM_MESSAGE);
	unbanMsg.AddInt32("im_what", IM_ROOM_UNBAN_PARTICIPANT);
	ChatCommand* unban = new ChatCommand("unban", unbanMsg, true, anyUser);
	unban->SetDesc("Undo a previous ban, allowing the user to rejoin (if they "
				   "still want to).");
	commands.AddItem("unban", unban);

	BMessage muteMsg(IM_MESSAGE);
	muteMsg.AddInt32("im_what", IM_ROOM_MUTE_PARTICIPANT);
	ChatCommand* mute = new ChatCommand("mute", muteMsg, true, roomUser);
	mute->SetDesc("Disallow a user from sending visible messages.");
	commands.AddItem("mute", mute);

	BMessage unmuteMsg(IM_MESSAGE);
	unmuteMsg.AddInt32("im_what", IM_ROOM_UNMUTE_PARTICIPANT);
	ChatCommand* unmute = new ChatCommand("unmute", unmuteMsg, true, roomUser);
	unmute->SetDesc("Restore a user's ability to send messages.");
	commands.AddItem("unmute", unmute);

	BMessage deafenMsg(IM_MESSAGE);
	deafenMsg.AddInt32("im_what", IM_ROOM_DEAFEN_PARTICIPANT);
	ChatCommand* deafen = new ChatCommand("deafen", deafenMsg, true, roomUser);
	deafen->SetDesc("Disallow a user from reading messages sent in the room.");
	commands.AddItem("deafen", deafen);

	BMessage undeafenMsg(IM_MESSAGE);
	undeafenMsg.AddInt32("im_what", IM_ROOM_UNDEAFEN_PARTICIPANT);
	ChatCommand* undeafen = new ChatCommand("undeafen", undeafenMsg, true, roomUser);
	undeafen->SetDesc("Restore a user's ability to receive messages.");
	commands.AddItem("undeafen", undeafen);

	BMessage inviteMsg(IM_MESSAGE);
	inviteMsg.AddInt32("im_what", IM_ROOM_SEND_INVITE);
	ChatCommand* invite = new ChatCommand("invite", inviteMsg, true, knownUser);
	invite->SetDesc("Invite a user to the current room.");
	commands.AddItem("invite", invite);

	BMessage helpMsg(APP_REQUEST_HELP);
	ChatCommand* help = new ChatCommand("help", helpMsg, false, List<int>());
	help->SetDesc("List all current commands, or get help for certain command.");
	commands.AddItem("help", help);

	BObjectList<BMessage> cmds;
	for (int i = 0; i < commands.CountItems(); i++) {
		ChatCommand* cmd = commands.ValueAt(i);
		BMessage* item = new BMessage();
		cmd->Archive(item);
		cmds.AddItem(item);
	}
	return cmds;
}


BObjectList<BMessage>
DefaultChatPopUpItems()
{
	BObjectList<BMessage> items;

	BMessage* leave = new BMessage(IM_MESSAGE);
	leave->AddInt32("im_what", IM_LEAVE_ROOM);
	BMessage* item = new BMessage(IM_MESSAGE);
	item->AddString("class", "BMenuItem");
	item->AddString("_label", "Leave chat");
	item->AddMessage("_msg", leave);
	item->AddBool("x_to_protocol", true);
	items.AddItem(item);
	
	return items;
}


BObjectList<BMessage>
DefaultUserPopUpItems()
{
	BObjectList<BMessage> items;

	BMessage* infoMsg = new BMessage(APP_USER_INFO);
	items.AddItem(_UserMenuItem("User info" B_UTF8_ELLIPSIS, infoMsg, 0,
		0, 0, false, false));

	BMessage* kickMsg = new BMessage(IM_MESSAGE);
	kickMsg->AddInt32("im_what", IM_ROOM_KICK_PARTICIPANT);
	items.AddItem(_UserMenuItem("Kick user", kickMsg, PERM_KICK, 0, 0,
		false, true));

	BMessage* banMsg = new BMessage(IM_MESSAGE);
	banMsg->AddInt32("im_what", IM_ROOM_BAN_PARTICIPANT);
	items.AddItem(_UserMenuItem("Ban user", banMsg, PERM_BAN, 0, 0, false,
		true));

	BMessage* muteMsg = new BMessage(IM_MESSAGE);
	muteMsg->AddInt32("im_what", IM_ROOM_MUTE_PARTICIPANT);
	items.AddItem(_UserMenuItem("Mute user", muteMsg, PERM_MUTE,
		PERM_WRITE, 0, false, true));

	BMessage* unmuteMsg = new BMessage(IM_MESSAGE);
	unmuteMsg->AddInt32("im_what", IM_ROOM_UNMUTE_PARTICIPANT);
	items.AddItem(_UserMenuItem("Unmute user", unmuteMsg, PERM_MUTE, 0,
		PERM_WRITE, false, true));

	BMessage* deafenMsg = new BMessage(IM_MESSAGE);
	deafenMsg->AddInt32("im_what", IM_ROOM_DEAFEN_PARTICIPANT);
	items.AddItem(_UserMenuItem("Deafen user", deafenMsg, PERM_DEAFEN,
		PERM_READ, 0, false, true));

	BMessage* undeafenMsg = new BMessage(IM_MESSAGE);
	undeafenMsg->AddInt32("im_what", IM_ROOM_UNDEAFEN_PARTICIPANT);
	items.AddItem(_UserMenuItem("Undeafen user", undeafenMsg, PERM_DEAFEN,
		0, PERM_READ, false, true));

	return items;
}


BMessage*
_UserMenuItem(const char* label, BMessage* msg, int32 user_perms,
			  int32 target_perms, int32 target_lacks, bool ignorePriority,
			  bool toProtocol)
{
	BMessage* item = new BMessage(IM_MESSAGE);
	item->AddString("class", "BMenuItem");
	item->AddString("_label", label);
	item->AddMessage("_msg", msg);

	item->AddInt32("x_perms", user_perms);
	item->AddInt32("x_target_perms", target_perms);
	item->AddInt32("x_target_antiperms", target_lacks);
	item->AddBool("x_priority", ignorePriority);
	item->AddBool("x_to_protocol", toProtocol);
	return item;
}


