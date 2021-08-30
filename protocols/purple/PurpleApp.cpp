/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * Copyright 1998-2021, Pidgin/Finch/libpurple contributors
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "PurpleApp.h"

#include <iostream>

#include <glib.h>
#include <libintl.h>
#include <libpurple/status.h>

#include <Alert.h>
#include <Catalog.h>
#include <Directory.h>
#include <File.h>
#include <Locale.h>
#include <MessageRunner.h>
#include <Path.h>
#include <Roster.h>

#include <ChatOMatic.h>
#include <ChatProtocolMessages.h>
#include <Flags.h>

#include "Purple.h"
#include "PurpleDialog.h"
#include "PurpleMessages.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp"


int
main(int arc, char** argv)
{

	PurpleApp app;
	app.Run();
	return 0;
}


PurpleApp::PurpleApp()
	:
	BApplication(PURPLE_SIGNATURE),
	fGloop(g_main_loop_new(NULL, false))
{
	init_gettext();
	if (init_libpurple() != B_OK)
		std::cerr << "libpurple initialization failed. Please report!\n";

	_GetProtocolsInfo();
	new BMessageRunner(this, new BMessage(G_MAIN_LOOP), 100000, -1);
	new BMessageRunner(this, new BMessage(CHECK_APP), 10000000, -1);
}


void
PurpleApp::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case IM_MESSAGE:
			ImMessage(msg);
			break;
		case PURPLE_REQUEST_PROTOCOL_COUNT:
		{
			int64 thread_id;
			if (msg->FindInt64("thread_id", &thread_id) != B_OK)	return;
			send_data(thread_id, fProtocols.CountItems(), NULL, 0);
			break;
		}
		case PURPLE_REQUEST_PROTOCOL_INFO:
		{
			int64 thread_id;
			if (msg->FindInt64("thread_id", &thread_id) != B_OK)	return;
			int32 index = msg->GetInt32("index", 0);
			ProtocolInfo* info = fProtocols.ItemAt(index);

			BMessage protoInfo;
			BMessage temps;
			temps.AddMessage("account", new BMessage(info->accountTemplate));
			temps.AddMessage("room", new BMessage(info->roomTemplate));
			protoInfo.AddMessage("templates", &temps);
			protoInfo.AddString("name", info->name);
			protoInfo.AddString("id", info->id);
			protoInfo.AddString("icon", info->iconName);

			SendMessage(thread_id, protoInfo);
			break;
		}
		case PURPLE_CONNECT_ACCOUNT:
		{
			_ParseAccountTemplate(msg);
			break;
		}
		case PURPLE_REGISTER_THREAD:
		{
			BString accName = msg->FindString("account_name");
			BString username = fAccounts.ValueFor(accName);
			int64 thread;
			if (username.IsEmpty() == true
					|| msg->FindInt64("thread_id", &thread) != B_OK)
				break;
			fAccountThreads.AddItem(username, thread);

			SendMessage(thread, _GetCommands(_AccountFromMessage(msg)));
			break;
		}
		case PURPLE_DISCONNECT_ACCOUNT:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			if (account == NULL)
				return;

			BString account_name = msg->FindString("account_name");
			const char* username = purple_account_get_username(account);
			fAccountThreads.RemoveItemFor(BString(username));
			fAccounts.RemoveItemFor(account_name);

			purple_account_disconnect(account);

			if (fAccountThreads.CountItems() == 0 || fAccounts.CountItems() == 0)
				Quit();
		}
		case CHECK_APP:
		{
			BRoster roster;
			if (roster.IsRunning(APP_SIGNATURE) == false)
				Quit();
			if (fAccountThreads.CountItems() == 0 || fAccounts.CountItems() == 0)
				Quit();
			break;
		}
		case G_MAIN_LOOP:
			g_main_context_iteration(g_main_loop_get_context(fGloop), false);
			break;
		default:
			BApplication::MessageReceived(msg);
	}
}


void
PurpleApp::ImMessage(BMessage* msg)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp ― Room moderation"
	switch (msg->FindInt32("im_what"))
	{
		case IM_SET_OWN_STATUS:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			UserStatus status = (UserStatus)msg->FindInt32("status");

			switch (status) {
				case STATUS_ONLINE:
					if (purple_account_is_disconnected(account)) {
						purple_account_set_enabled(account, PURPLE_UI_ID, true);
						purple_account_connect(account);
					}
					break;
				case STATUS_OFFLINE: {
					SendMessage(account, BMessage(PURPLE_SHUTDOWN_ADDON));
					purple_account_disconnect(account);
					break;
				}
			}

			PurpleStatusPrimitive prim = cardie_status_to_purple(status);
			const char* primId = purple_primitive_get_id_from_type(prim);

			std::cout << "purple setting status to " << primId << "…\n";
			purple_account_set_status(account, primId, true);
			break;
		}
		case IM_SET_OWN_NICKNAME:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			const char* nick;
			if (msg->FindString("user_name", &nick) == B_OK)
				purple_account_set_public_alias(account, nick,
					NULL, &callback_set_public_alias_failure);
			break;
		}
		case IM_SEND_MESSAGE:
		{
			BString body;
			if (msg->FindString("body", &body) != B_OK)	break;

			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			PurpleConvIm* im = purple_conversation_get_im_data(conv);

			if (chat != NULL)
				purple_conv_chat_send(chat, body.String());
			else if (im != NULL)
				purple_conv_im_send(im, body.String());
			break;
		}
		case IM_CREATE_CHAT:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			const char* user_id = msg->FindString("user_id");
			if (user_id == NULL || purple_find_buddy(account, user_id) == NULL)
				break;

			purple_conversation_new(PURPLE_CONV_TYPE_IM, account, user_id);

			BMessage created(IM_MESSAGE);
			created.AddInt32("im_what", IM_CHAT_CREATED);
			created.AddString("user_id", user_id);
			created.AddString("chat_id", user_id);
			SendMessage(account, created);
			break;
		}
		case IM_JOIN_ROOM:
		case IM_CREATE_ROOM:
		{
			PurpleConnection* conn = _ConnectionFromMessage(msg);
			if (msg->GetBool("fromRoomlist", false) == false)
				serv_join_chat(conn, _ParseRoomTemplate(msg));
			else
				serv_join_chat(conn, _FindRoomlistComponents(msg));
			break;
		}
		case IM_LEAVE_ROOM:
		{
			PurpleConversation* conv = _ConversationFromMessage(msg);
			BString chat_id = msg->FindString("chat_id");
			if (conv == NULL || chat_id.IsEmpty() == true) break;

			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			if (chat != NULL)
				serv_chat_leave(purple_conversation_get_gc(conv),
					purple_conv_chat_get_id(chat));
			break;
		}
		case IM_GET_ROOM_METADATA:
		{
			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);

			BMessage meta(IM_MESSAGE);
			meta.AddInt32("im_what", IM_ROOM_METADATA);
			meta.AddString("chat_id", purple_conversation_get_name(conv));
			meta.AddString("chat_name", purple_conversation_get_title(conv));
			meta.AddInt32("room_default_flags",
				0 | ROOM_LOG_LOCALLY | ROOM_POPULATE_LOGS | ROOM_NOTIFY_DM);
			if (chat != NULL)
				meta.AddString("subject", purple_conv_chat_get_topic(chat));
			SendMessage(purple_conversation_get_account(conv), meta);
			break;
		}
		case IM_GET_ROOM_PARTICIPANTS:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			PurpleConvIm* im = purple_conversation_get_im_data(conv);
			if (chat == NULL && im == NULL) break;

			BStringList user_ids;
			if (im != NULL)
				user_ids.Add(BString(purple_conversation_get_name(conv)));
			else {
				GList* users = purple_conv_chat_get_users(chat);
				for (int i = 0; users != NULL; users = users->next) {
					PurpleConvChatBuddy* user = (PurpleConvChatBuddy*)users->data;

					const char* user_name = purple_conv_chat_cb_get_name(user);
					if (is_own_user(account, user_name) == false)
						user_ids.Add(BString(user_name));
				}
			}

			BMessage parts(IM_MESSAGE);
			parts.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
			parts.AddString("chat_id", purple_conversation_get_name(conv));
			parts.AddStrings("user_id", user_ids);
			SendMessage(purple_conversation_get_account(conv), parts);
			break;
		}
		case IM_GET_ROOM_DIRECTORY:
		{
			PurpleConnection* conn = _ConnectionFromMessage(msg);
			if (conn != NULL)
				PurpleRoomlist* list = purple_roomlist_get_list(conn);
			break;
		}
		case IM_GET_ROSTER:
		{
			PurpleAccount* account = _AccountFromMessage(msg);

			BStringList user_ids;
			GSList* buddies = purple_blist_get_buddies();
			for (int i = 0; buddies != NULL; buddies = buddies->next) {
				PurpleBuddy* buddy = (PurpleBuddy*)buddies->data;
				if (purple_buddy_get_account(buddy) == account)
					user_ids.Add(BString(purple_buddy_get_name(buddy)));
			}

			BMessage roster(IM_MESSAGE);
			roster.AddInt32("im_what", IM_ROSTER);
			roster.AddStrings("user_id", user_ids);
			SendMessage(_AccountFromMessage(msg), roster);
			break;
		}
		case IM_ROSTER_ADD_CONTACT:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			BString user_id = msg->FindString("user_id");
			const char* user_name = msg->FindString("user_name");
			if (user_id.IsEmpty() == true || account == NULL) break;

			PurpleBuddy* buddy =
				purple_buddy_new(account, user_id.String(), user_name);

			purple_blist_add_buddy(buddy, NULL, NULL, NULL);
			purple_account_add_buddy_with_invite(account, buddy, NULL);
			update_buddy(buddy_cache(buddy), user_id, BString(user_name));
			break;
		}
		case IM_ROSTER_REMOVE_CONTACT:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			BString user_id = msg->FindString("user_id");
			if (user_id.IsEmpty() == true || account == NULL) break;

			PurpleBuddy* buddy = purple_find_buddy(account, user_id.String());
			if (buddy == NULL) return;

			purple_blist_remove_buddy(buddy);
			purple_account_remove_buddy(account, buddy, NULL);
			BEntry(buddy_cache(buddy)).Remove();
			break;
		}
		case IM_ROSTER_EDIT_CONTACT:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			BString user_id = msg->FindString("user_id");
			BString user_name = msg->FindString("user_name");
			PurpleBuddy* buddy = purple_find_buddy(account, user_id.String());
			if (buddy == NULL) return;

			if (user_name.IsEmpty() == false) {
				purple_blist_alias_buddy(buddy, user_name.String());
				update_buddy(buddy_cache(buddy), user_id, BString(user_name));
			}
			break;
		}
		case IM_GET_EXTENDED_CONTACT_INFO:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			BString user_id = msg->FindString("user_id");
			if (user_id.IsEmpty() == true || account == NULL) break;

			PurpleBuddy* buddy = purple_find_buddy(account, user_id.String());
			if (buddy == NULL) return;

			BString user_name = purple_buddy_get_alias(buddy);
			if (user_name.IsEmpty() == true)
				user_name = purple_buddy_get_server_alias(buddy);

			BMessage info(IM_MESSAGE);
			info.AddInt32("im_what", IM_EXTENDED_CONTACT_INFO);
			info.AddString("user_id", user_id);
			if (user_name.IsEmpty() == false)
				info.AddString("user_name", user_name);
			SendMessage(account, info);
			break;
		}
		case IM_SET_ROOM_SUBJECT:
		{
			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			BString subject;

			if (chat != NULL || msg->FindString("subject", &subject) == B_OK)
				purple_conv_chat_set_topic(chat, NULL, subject.String());
			break;
		}
		case IM_ROOM_SEND_INVITE:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			BString user_id = msg->FindString("user");
			BString body = msg->FindString("body");

			if (chat == NULL || user_id.IsEmpty() == true)
				break;
			if (body.IsEmpty() == true)
				body = "(Invite)";

			purple_conv_chat_invite_user(chat, user_id.String(), body.String(),
				false);
			break;
		}
		case IM_ROOM_INVITE_ACCEPT:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			BString key(purple_account_get_username(account));
			key = key.Append("-invite-").Append(msg->FindString("chat_id"));
			GHashTable* data = fInviteList.ValueFor(key);

			if (data != NULL)
				serv_join_chat(purple_account_get_connection(account), data);
			break;
		}
		case IM_ROOM_INVITE_REFUSE:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			BString key(purple_account_get_username(account));
			key = key.Append("-invite-").Append(msg->FindString("chat_id"));
			GHashTable* data = fInviteList.ValueFor(key);

			if (data != NULL)
				serv_reject_chat(purple_account_get_connection(account), data);
			break;
		}
		case IM_ROOM_KICK_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** This protocol doesn't support kicking. "
					"Send them a strongly worded e-mail.\n"));
			break;
		}
		case IM_ROOM_BAN_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** Banning won't work with this protocol. "
					"Try being mean instead.\n"));
			break;
		}
		case IM_ROOM_UNBAN_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** You can't undo what was once done… "
					"at least with this protocol.\n"));
			break;
		}
		case IM_ROOM_MUTE_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** This protocol left the duct-tape at home― "
					"we have nothing to put over their mouth!\n"));
			break;
		}
		case IM_ROOM_UNMUTE_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** This protocol can't exactly unmute a user, "
					"let alone make an omlett.\n"));
			break;
		}
		case IM_ROOM_DEAFEN_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** This protocol doesn't support deafening, "
					"but spamming the chat should be a good substitute. :^)\n"));
			break;
		}
		case IM_ROOM_UNDEAFEN_PARTICIPANT:
		{
			_SendSysText(_ConversationFromMessage(msg),
				B_TRANSLATE("** This protocol is particularly self-concious,"
					" and prefers that this person not see its chats.\n"));
			break;
		}
		case PURPLE_CHAT_COMMAND:
		{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp ― Command errors"
			PurpleConversation* conv = _ConversationFromMessage(msg);
			BString cmd;
			if (conv == NULL || msg->FindString("cmd_name", &cmd) != B_OK)
				break;
			cmd << " " << msg->FindString("misc_str");

			const char* cmdline = cmd.String();
			const char* escape = g_markup_escape_text(cmd, -1);

			char* error = NULL;
			PurpleCmdStatus status =
				purple_cmd_do_command(conv, cmdline, escape, &error);

			BMessage errorMsg(IM_MESSAGE);
			errorMsg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
			errorMsg.AddString("chat_id", msg->FindString("chat_id"));
			BString errorBody;

			switch (status)
			{
				case PURPLE_CMD_STATUS_FAILED:
					errorBody = B_TRANSLATE("** Command failed %err%\n");
					break;
				case PURPLE_CMD_STATUS_NOT_FOUND:
					errorBody = B_TRANSLATE("** Command not found %err%\n");
					break;
				case PURPLE_CMD_STATUS_WRONG_ARGS:
					errorBody = B_TRANSLATE("** Invalid arguments to command %err%\n");
					break;
				case PURPLE_CMD_STATUS_WRONG_PRPL:
					errorBody = B_TRANSLATE("** Command isn't useful in this chat %err%\n");
					break;
				default:
					errorBody = B_TRANSLATE("** Command error %err%\n");
			}
			if (status != PURPLE_CMD_STATUS_OK) {
				errorBody.ReplaceAll("%err%", _tr(error));
				_SendSysText(conv, errorBody.String());
			}
			break;
		}
		default:
			std::cout << "IM_MESSAGE unhandled by Purple:\n";
			msg->PrintToStream();
	}
}


void
PurpleApp::SendMessage(thread_id thread, BMessage msg)
{
	ssize_t size = msg.FlattenedSize();
	char buffer[size];

	send_data(thread, size, NULL, 0);
	msg.Flatten(buffer, size);
	send_data(thread, 0, buffer, size);
}


void
PurpleApp::SendMessage(PurpleAccount* account, BMessage msg)
{
	const char* username = purple_account_get_username(account);
	thread_id thread = fAccountThreads.ValueFor(BString(username));
	if (thread > 0)
		SendMessage(thread, msg);
	else
		std::cerr << "Failed to send message: " << msg.what << std::endl;
}


void
PurpleApp::_SendSysText(PurpleConversation* conv, const char* body)
{
	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msg.AddString("chat_id", purple_conversation_get_name(conv));
	msg.AddString("body", body);
	((PurpleApp*)be_app)->SendMessage(purple_conversation_get_account(conv), msg);
}


void
PurpleApp::_GetProtocolsInfo()
{
	GList* listIter = purple_plugins_get_protocols();
	for (int i = 0; listIter; listIter = listIter->next) {
		PurplePlugin* plugin = (PurplePlugin*)listIter->data;
		if (plugin)
			_SaveProtocolInfo(plugin);
	}
}


void
PurpleApp::_SaveProtocolInfo(PurplePlugin* plugin)
{
	ProtocolInfo* proto = new ProtocolInfo;
	proto->id = purple_plugin_get_id(plugin);
	proto->name = purple_plugin_get_name(plugin);

	PurplePluginProtocolInfo* info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin);
	proto->accountTemplate = _GetAccountTemplate(info);
	proto->roomTemplate = _GetRoomTemplate(info);
	if (info->list_icon != NULL)
		proto->iconName = info->list_icon(NULL, NULL);
	fProtocols.AddItem(proto);
}


BMessage
PurpleApp::_GetAccountTemplate(PurplePluginProtocolInfo* info)
{
	BMessage temp;

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp ― Account template"

	// Add a "username" setting, if not explicitly specified
	GList* prefIter = info->protocol_options;
	for (int i = 0; prefIter != NULL; prefIter = prefIter->next) {
		PurpleAccountOption* pref = (PurpleAccountOption*)prefIter->data;

		if (pref->pref_name == BString("username"))
			break;
		else if (prefIter->next == NULL) {
			BMessage setting;
			setting.AddString("name", "username");
			setting.AddString("description", B_TRANSLATE("Username:"));
			setting.AddString("error",
				B_TRANSLATE("A username needs to be specified!"));
			setting.AddInt32("type", B_STRING_TYPE);
			temp.AddMessage("setting", &setting);
		}
	}

	// Add any UserSplits (that is, parts of the protocol's "username" format)
	GList* splitIter = info->user_splits;
	for (int i = 0; splitIter != NULL; splitIter = splitIter->next)
	{
		PurpleAccountUserSplit* split = (PurpleAccountUserSplit*)splitIter->data;
		BMessage setting;
		setting.AddString("name", "username_split");
		setting.AddString("description", BString(_tr(split->text)).Append(":"));
		setting.AddString("default", split->default_value);
		setting.AddInt32("type", B_STRING_TYPE);

		BString error(B_TRANSLATE("%name% needs to be specified."));
		error.ReplaceAll("%name%", _tr(split->text));
		setting.AddString("error", error);

		temp.AddMessage("setting", &setting);
	}

	// Password setting
	BMessage passwd;
	passwd.AddString("name", "password");
	passwd.AddString("description", B_TRANSLATE("Password:"));
	passwd.AddInt32("type", B_STRING_TYPE);
	passwd.AddBool("is_secret", true);
	temp.AddMessage("setting", &passwd);

	// Whatever custom settings the protocol might like!
	prefIter = info->protocol_options;
	for (int i = 0; prefIter != NULL; prefIter = prefIter->next)
	{
		PurpleAccountOption* pref = (PurpleAccountOption*)prefIter->data;
		PurplePrefType type = pref->type;
		int32 bType;

		BMessage setting;
		setting.AddString("name", pref->pref_name);
		BString description = BString(_tr(pref->text)).Append(":");

		switch (type)
		{
			case PURPLE_PREF_BOOLEAN:
			{
				description = _tr(pref->text);
				bType = B_BOOL_TYPE;
				setting.AddBool("default", pref->default_value.boolean);
				break;
			}
			case PURPLE_PREF_INT:
			{
				bType = B_INT32_TYPE;
				setting.AddInt32("default", pref->default_value.integer);
				break;
			}
			case PURPLE_PREF_PATH_LIST:
			case PURPLE_PREF_STRING_LIST:
			{
				bType = B_STRING_TYPE;
				GList* list = pref->default_value.list;
				for (int i = 0; list != NULL; list = list->next) {
					PurpleKeyValuePair* pair = (PurpleKeyValuePair*)list->data;
					setting.AddString("valid_value", pair->key);
					if (pair->value ==
							purple_account_option_get_default_list_value(pref))
						temp.AddString(pref->pref_name, pair->key);
				}
				break;
			}
			default:
				bType = B_STRING_TYPE;
				setting.AddString("default", pref->default_value.string);
		}

		if (pref->masked)
			setting.AddBool("is_hidden", true);
		setting.AddInt32("type", bType);
		setting.AddString("description", description);
		temp.AddMessage("setting", &setting);
	}
	return temp;
}


BMessage
PurpleApp::_GetRoomTemplate(PurplePluginProtocolInfo* info)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp ― Room template"

	BMessage settings;
	if (info->chat_info == NULL) {
		settings.AddString("name", "chat_id");
		settings.AddString("description", B_TRANSLATE("Room ID"));
		settings.AddInt32("type", B_STRING_TYPE);
		return settings;
	}

	GList* prefs = info->chat_info(NULL);

	for (int i = 0; prefs != NULL; prefs = prefs->next) {
		BMessage setting;
		proto_chat_entry* pref = (proto_chat_entry*)prefs->data;

		setting.AddString("name", pref->identifier);
		setting.AddString("description", _tr(pref->label));

		if (pref->required) {
			BString error(B_TRANSLATE("%name% is necessary."));
			error.ReplaceAll("%name%", pref->identifier);
			setting.AddString("error", error);
		}
		if (pref->secret)
			setting.AddBool("is_secret", true);
		if (pref->is_int)
			setting.AddInt32("type", B_INT32_TYPE);
		else
			setting.AddInt32("type", B_STRING_TYPE);

		settings.AddMessage("setting", &setting);
	}
	return settings;
}


BMessage
PurpleApp::_GetCommands(PurpleAccount* account)
{
	PurpleConversation* conv = purple_conversation_new(PURPLE_CONV_TYPE_ANY,
		account, NULL);

	BMessage cmdMsgs(PURPLE_REGISTER_COMMANDS);
	GList* cmds = purple_cmd_list(conv);

	for (int i = 0; cmds != NULL; cmds = cmds->next) {
		const char* cmd_name = (const char*)cmds->data;
		if (cmd_name == NULL) break;;

		BMessage cmdMsg;
		cmdMsg.AddString("class", "ChatCommand");
		cmdMsg.AddString("_name", cmd_name);
		cmdMsg.AddBool("_proto", true);
		cmdMsg.AddInt32("_argtype", 0);

		BString helpString;
		GList* helps = purple_cmd_help(NULL, cmd_name);
		for (int j = 0; helps != NULL; helps = helps->next)
			helpString << (const char*)helps->data;
		cmdMsg.AddString("_desc", helpString);

		BMessage cmdActionMsg(IM_MESSAGE);
		cmdActionMsg.AddInt32("im_what", PURPLE_CHAT_COMMAND);
		cmdActionMsg.AddString("cmd_name", cmd_name);
		cmdMsg.AddMessage("_msg", &cmdActionMsg);

		cmdMsgs.AddMessage("command", &cmdMsg);
	}
	return cmdMsgs;
}


void
PurpleApp::_ParseAccountTemplate(BMessage* settings)
{
	PurplePlugin* plugin = _PluginFromMessage(settings);
	PurplePluginProtocolInfo* info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin);
	const char* protoId = settings->FindString("protocol");

	if (plugin == NULL || info == NULL)
		return;

	// Fetch and cobble together the username & password
	BString username, password;
	settings->FindString("username", &username);
	settings->FindString("password", &password);

	GList* splitIter = info->user_splits;
	for (int i = 0; splitIter != NULL; splitIter = splitIter->next)
	{
		PurpleAccountUserSplit* split = (PurpleAccountUserSplit*)splitIter->data;
		username << split->field_sep;

		BString opt;
		if (settings->FindString("username_split", i, &opt) == B_OK)
			username << opt;
		else
			username << split->default_value;
		i++;
	}

	// Create/fetch the account itself
	PurpleAccount* account = purple_accounts_find(username.String(), protoId);
	if (account == NULL) {
		account = purple_account_new(username.String(), protoId);
		purple_accounts_add(account);
	}

	purple_account_set_password(account, password.String());

	// Set all protocol settings
	GList* prefIter = info->protocol_options;
	for (int i = 0; prefIter != NULL; prefIter = prefIter->next)
	{
		PurpleAccountOption* pref = (PurpleAccountOption*)prefIter->data;
		PurplePrefType type = pref->type;

		switch (type)
		{
			case PURPLE_PREF_BOOLEAN:
			{
				bool value;
				if (settings->FindBool(pref->pref_name, &value) == B_OK)
					purple_account_set_bool(account, pref->pref_name, value);
				break;
			}
			case PURPLE_PREF_INT:
			{
				int32 value;
				if (settings->FindInt32(pref->pref_name, &value) == B_OK)
					purple_account_set_int(account, pref->pref_name, value);
				break;
			}
			case PURPLE_PREF_PATH_LIST:
			case PURPLE_PREF_STRING_LIST:
			{
				GList* list = pref->default_value.list;
				BString value = settings->FindString(pref->pref_name);

				for (int i = 0; list != NULL; list = list->next) {
					PurpleKeyValuePair* pair = (PurpleKeyValuePair*)list->data;
					if (pair->key == value) {
						purple_account_set_string(account, pref->pref_name,
							(const char*)pair->value);
						break;
					}
				}
				break;
			}
			default:
				BString value;
				if (settings->FindString(pref->pref_name, &value) == B_OK)
					purple_account_set_string(account, pref->pref_name,
						value.String());
		}
	}
	fAccounts.AddItem(settings->FindString("account_name"), username);
	purple_account_set_enabled(account, PURPLE_UI_ID, true);
	purple_account_set_ui_bool(account, PURPLE_UI_ID, "auto-login", false);
}


GHashTable*
PurpleApp::_ParseRoomTemplate(BMessage* msg)
{
	PurplePlugin* plugin = _PluginFromMessage(msg);
	PurplePluginProtocolInfo* info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin);

	if (info->chat_info == NULL && info->chat_info_defaults != NULL)
		return info->chat_info_defaults(_ConnectionFromMessage(msg),
			msg->FindString("chat_id"));
	else if (info->chat_info == NULL)
		return NULL;

	GHashTable* table
		= g_hash_table_new_full(g_str_hash, g_str_equal, NULL, g_free);

	GList* prefs = info->chat_info(NULL);
	for (int i = 0; prefs != NULL; prefs = prefs->next) {
		BString setting;
		proto_chat_entry* pref = (proto_chat_entry*)prefs->data;
		if (msg->FindString(pref->identifier, &setting) == B_OK)
			g_hash_table_insert(table, (void*)pref->identifier, g_strdup(setting.String()));
	}
	return table;
}


GHashTable*
PurpleApp::_FindRoomlistComponents(BMessage* msg)
{
	PurpleRoomlist* list = fRoomlists.ValueFor(_AccountFromMessage(msg));
	if (list == NULL || list->rooms == NULL)
		return NULL;

	// Find the room by iterating over the account's roomlist
	GList* rooms;
	PurpleRoomlistRoom* room = NULL;
	for (rooms = list->rooms; rooms; rooms = rooms->next) {
		PurpleRoomlistRoom* testRoom = (PurpleRoomlistRoom*)rooms->data;
		const char* name = purple_roomlist_room_get_name(testRoom);

		if (strcmp(msg->FindString("chat_id"), name) == 0) {
			room = testRoom;
			break;
		}
	}

	GList* listFields, *roomFields;
	GHashTable* components = g_hash_table_new(g_str_hash, g_str_equal);
	g_hash_table_replace(components, (void*)"name", room->name);
	for (listFields = list->fields, roomFields = room->fields;
		 listFields && roomFields;
		 listFields = listFields->next, roomFields = roomFields->next)
	{
		PurpleRoomlistField* listField = (PurpleRoomlistField*)listFields->data;
		g_hash_table_replace(components, listField->name, roomFields->data);
	}
	return components;
}


PurplePlugin*
PurpleApp::_PluginFromMessage(BMessage* msg)
{
	return purple_plugins_find_with_id(msg->FindString("protocol"));
}


PurpleConnection*
PurpleApp::_ConnectionFromMessage(BMessage* msg)
{
	return purple_account_get_connection(_AccountFromMessage(msg));
}


PurpleAccount*
PurpleApp::_AccountFromMessage(BMessage* msg)
{
	BString protocol = msg->FindString("protocol");
	BString username = fAccounts.ValueFor(msg->FindString("account_name"));
	return purple_accounts_find(username.String(), protocol.String());
}


PurpleConversation*
PurpleApp::_ConversationFromMessage(BMessage* msg)
{
	PurpleAccount* account = _AccountFromMessage(msg);
	BString chat_id = msg->FindString("chat_id");

	return purple_find_conversation_with_account(PURPLE_CONV_TYPE_ANY,
		chat_id.String(), account);
}


status_t
init_libpurple()
{
	init_ui_ops();

	purple_util_set_user_dir(purple_cache());

	BString cachePlugin = BString(purple_cache()).Append("/plugins/");
	purple_plugins_add_search_path(cachePlugin.String());
	purple_plugins_add_finddir(B_USER_LIB_DIRECTORY);
	purple_plugins_add_finddir(B_USER_NONPACKAGED_LIB_DIRECTORY);
	purple_plugins_add_finddir(B_SYSTEM_NONPACKAGED_LIB_DIRECTORY);

	BPath ssl;
	if (find_directory(B_SYSTEM_DATA_DIRECTORY, &ssl) == B_OK) {
		ssl.Append("ssl");
		purple_certificate_add_ca_search_path(ssl.Path());
	}

	purple_debug_set_enabled(DEBUG_ENABLED);

	if (!purple_core_init(PURPLE_UI_ID))
		return B_ERROR;

	purple_set_blist(purple_blist_new());
	purple_blist_load();

	init_signals();
	return B_OK;
}


void
init_gettext()
{
	// Spoof the current language
	BLocale locale;
	BLanguage lang;
	if (locale.GetLanguage(&lang) == B_OK)
		setenv("LC_MESSAGES", lang.Code(), 1);

	bindtextdomain("pidgin", "/boot/system/data/locale");
	bind_textdomain_codeset("pidgin", "UTF-8");
	textdomain("pidgin");

	setlocale(LC_MESSAGES, NULL);
}


static PurpleEventLoopUiOps _ui_op_eventloops =
{
	g_timeout_add,
	g_source_remove,
	ui_op_input_add,
	g_source_remove,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


static PurpleConnectionUiOps _ui_op_connection =
{
	NULL,
	NULL,
	ui_op_disconnected,
	NULL,
	NULL,
	NULL,
	NULL,
	ui_op_report_disconnect_reason,
	NULL,
	NULL,
	NULL
};


static PurpleConversationUiOps _ui_op_conversation =
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	ui_op_chat_rename_user,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};


static PurpleRoomlistUiOps _ui_op_roomlist =
{
	NULL,
	NULL,
	NULL,
	ui_op_add_room,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


static PurpleRequestUiOps _ui_op_request =
{
	ui_op_request_input,
	ui_op_request_choice,
	ui_op_request_action,
	ui_op_request_fields,
	ui_op_request_file,
	NULL,
	ui_op_request_folder,
	ui_op_request_action_with_icon,
	NULL,
	NULL,
	NULL
};


static PurpleNotifyUiOps _ui_op_notify =
{
	ui_op_notify_message,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


void
init_ui_ops()
{
	purple_eventloop_set_ui_ops(&_ui_op_eventloops);
	purple_connections_set_ui_ops(&_ui_op_connection);
	purple_conversations_set_ui_ops(&_ui_op_conversation);
	purple_roomlist_set_ui_ops(&_ui_op_roomlist);
	purple_request_set_ui_ops(&_ui_op_request);
	purple_notify_set_ui_ops(&_ui_op_notify);
}


void
init_signals()
{
	int handle;

	purple_signal_connect(purple_connections_get_handle(), "connection-error",
		&handle, PURPLE_CALLBACK(signal_connection_error), NULL);

	purple_signal_connect(purple_accounts_get_handle(), "account-signed-on",
		&handle, PURPLE_CALLBACK(signal_account_signed_on), NULL);
	purple_signal_connect(purple_accounts_get_handle(), "account-error-changed",
		&handle, PURPLE_CALLBACK(signal_account_error_changed), NULL);
	purple_signal_connect(purple_accounts_get_handle(), "account-alias-changed",
		&handle, PURPLE_CALLBACK(signal_account_alias_changed), NULL);
	purple_signal_connect(purple_accounts_get_handle(), "account-status-changed",
		&handle, PURPLE_CALLBACK(signal_account_status_changed), NULL);

	purple_signal_connect(purple_blist_get_handle(), "blist-node-added",
		&handle, PURPLE_CALLBACK(signal_blist_node_added), NULL);
	purple_signal_connect(purple_blist_get_handle(), "blist-node-removed",
		&handle, PURPLE_CALLBACK(signal_blist_node_removed), NULL);
	purple_signal_connect(purple_blist_get_handle(), "buddy-status-changed",
		&handle, PURPLE_CALLBACK(signal_buddy_status_changed), NULL);
	purple_signal_connect(purple_blist_get_handle(), "buddy-icon-changed",
		&handle, PURPLE_CALLBACK(signal_buddy_icon_changed), NULL);

	purple_signal_connect(purple_conversations_get_handle(), "chat-joined",
		&handle, PURPLE_CALLBACK(signal_chat_joined), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-left",
		&handle, PURPLE_CALLBACK(signal_chat_left), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "received-chat-msg",
		&handle, PURPLE_CALLBACK(signal_received_chat_msg), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "received-im-msg",
		&handle, PURPLE_CALLBACK(signal_received_chat_msg), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "sent-chat-msg",
		&handle, PURPLE_CALLBACK(signal_sent_chat_msg), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "sent-im-msg",
		&handle, PURPLE_CALLBACK(signal_sent_im_msg), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-topic-changed",
		&handle, PURPLE_CALLBACK(signal_chat_topic_changed), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-buddy-joined",
		&handle, PURPLE_CALLBACK(signal_chat_buddy_joined), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-buddy-left",
		&handle, PURPLE_CALLBACK(signal_chat_buddy_left), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-invited",
		&handle, PURPLE_CALLBACK(signal_chat_invited), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-buddy-flags",
		&handle, PURPLE_CALLBACK(signal_chat_buddy_flags), NULL);
}


static void
signal_connection_error(PurpleConnection* gc, PurpleConnectionError err,
	const gchar* desc)
{
	std::cerr << "Connection failed: " << (const char*)desc << std::endl;
}


static void
signal_account_signed_on(PurpleAccount* account)
{
	BMessage readyMsg(IM_MESSAGE);
	readyMsg.AddInt32("im_what", IM_PROTOCOL_READY);
	PurpleApp* app = (PurpleApp*)be_app;
	((PurpleApp*)be_app)->SendMessage(account, readyMsg);

	BString username = purple_account_get_username(account);
	BString display = purple_account_get_name_for_display(account);

	send_own_info(account);
	load_account_buddies(account);

	BMessage status(IM_MESSAGE);
	status.AddInt32("im_what", IM_OWN_STATUS_SET);
	status.AddInt32("status", (int32)STATUS_ONLINE);
	((PurpleApp*)be_app)->SendMessage(account, status);

	((PurpleApp*)be_app)->fUserNicks.AddItem(username, display);
}


static void
signal_account_error_changed(PurpleAccount* account,
	const PurpleConnectionErrorInfo* old_error,
	const PurpleConnectionErrorInfo* current_error)
{
	if (current_error == NULL)	return;

	BMessage error(IM_ERROR);
	error.AddString("error", purple_connection_error_name(current_error));
	error.AddString("detail", current_error->description);
	((PurpleApp*)be_app)->SendMessage(account, error);
}


static void
signal_account_alias_changed(PurpleAccount* account, const char* old)
{
	BMessage own(IM_MESSAGE);
	own.AddInt32("im_what", IM_OWN_NICKNAME_SET);
	own.AddString("user_name", purple_account_get_alias(account));
	((PurpleApp*)be_app)->SendMessage(account, own);
}


static void
signal_account_status_changed(PurpleAccount* account, PurpleStatus* old,
	PurpleStatus* cur)
{
	BMessage own(IM_MESSAGE);
	own.AddInt32("im_what", IM_OWN_STATUS_SET);
	own.AddInt32("status", purple_status_to_cardie(cur));
	((PurpleApp*)be_app)->SendMessage(account, own);
}


static void
signal_blist_node_added(PurpleBlistNode* node)
{
	PurpleBuddy* buddy;
	if (PURPLE_BLIST_NODE_IS_BUDDY(node))
		buddy = PURPLE_BUDDY(node);
	else if (PURPLE_BLIST_NODE_IS_CONTACT(node))
		buddy = purple_contact_get_priority_buddy(PURPLE_CONTACT(node));
	else
		return;

	BMessage add(IM_MESSAGE);
	add.AddInt32("im_what", IM_ROSTER);
	add.AddString("user_id", purple_buddy_get_name(buddy));
	((PurpleApp*)be_app)->SendMessage(purple_buddy_get_account(buddy), add);

	BString alias = purple_buddy_get_local_alias(buddy);
	if (alias.IsEmpty() == true)
		alias.SetTo(purple_buddy_get_server_alias(buddy));

	BMessage name(IM_MESSAGE);
	name.AddInt32("im_what", IM_CONTACT_INFO);
	name.AddString("user_id", purple_buddy_get_name(buddy));
	if (alias.IsEmpty() == false)
		name.AddString("user_name", alias);
	((PurpleApp*)be_app)->SendMessage(purple_buddy_get_account(buddy), name);
}


static void
signal_blist_node_removed(PurpleBlistNode* node)
{
	PurpleBuddy* buddy;
	if (PURPLE_BLIST_NODE_IS_BUDDY(node))
		buddy = PURPLE_BUDDY(node);
	else if (PURPLE_BLIST_NODE_IS_CONTACT(node))
		buddy = purple_contact_get_priority_buddy(PURPLE_CONTACT(node));
	else
		return;

	BMessage rem(IM_MESSAGE);
	rem.AddInt32("im_what", IM_ROSTER_CONTACT_REMOVED);
	rem.AddString("user_id", purple_buddy_get_name(buddy));
	((PurpleApp*)be_app)->SendMessage(purple_buddy_get_account(buddy), rem);
}


static void
signal_buddy_status_changed(PurpleBuddy* buddy, PurpleStatus* old_status,
	PurpleStatus* status)
{
	BMessage note(IM_MESSAGE);
	note.AddInt32("im_what", IM_USER_STATUS_SET);
	note.AddInt32("status", purple_status_to_cardie(status));
	note.AddString("user_id", purple_buddy_get_name(buddy));
	((PurpleApp*)be_app)->SendMessage(purple_buddy_get_account(buddy), note);
}


static void
signal_buddy_icon_changed(PurpleBuddy* buddy)
{
	entry_ref ref;
	if (get_ref_for_path(purple_buddy_icon_get_full_path(
			purple_buddy_get_icon(buddy)), &ref) != B_OK)
		return;

	BMessage avatar(IM_MESSAGE);
	avatar.AddInt32("im_what", IM_USER_AVATAR_SET);
	avatar.AddString("user_id", purple_buddy_get_name(buddy));
	avatar.AddRef("ref", &ref);
	((PurpleApp*)be_app)->SendMessage(purple_buddy_get_account(buddy), avatar);
}


static void
signal_chat_joined(PurpleConversation* conv)
{
	BMessage join(IM_MESSAGE);
	join.AddInt32("im_what", IM_ROOM_JOINED);
	join.AddString("chat_id", purple_conversation_get_name(conv));

	PurpleAccount* account = purple_conversation_get_account(conv);
	((PurpleApp*)be_app)->SendMessage(account, join);
}


static void
signal_chat_left(PurpleConversation* conv)
{
	BMessage left(IM_MESSAGE);
	left.AddInt32("im_what", IM_ROOM_LEFT);
	left.AddString("chat_id", purple_conversation_get_name(conv));

	PurpleAccount* account = purple_conversation_get_account(conv);
	((PurpleApp*)be_app)->SendMessage(account, left);
}


static void
signal_received_chat_msg(PurpleAccount* account, char* sender, char* message,
	PurpleConversation* conv, PurpleMessageFlags flags)
{
	if (is_own_user(account, sender) == true)
		return;

	BString chat_id = BString(purple_conversation_get_name(conv));
	if (chat_id.IsEmpty() == true)
		chat_id = sender;

	PurpleConvChat* chat = purple_conversation_get_chat_data(conv);

	BMessage msg(IM_MESSAGE);
	msg.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	msg.AddString("chat_id", chat_id);
	msg.AddString("user_id", sender);
	msg.AddString("body", purple_unescape_text(message));
	((PurpleApp*)be_app)->SendMessage(account, msg);
}


static void
signal_sent_chat_msg(PurpleAccount* account, const char* message, int conv_id)
{
	PurpleConnection* gc = purple_account_get_connection(account);
	PurpleConversation* conv = purple_find_chat(gc, conv_id);

	BMessage sent(IM_MESSAGE);
	sent.AddInt32("im_what", IM_MESSAGE_SENT);
	sent.AddString("chat_id", purple_conversation_get_name(conv));
	sent.AddString("user_id", purple_account_get_username(account));
	sent.AddString("body", purple_unescape_text(message));
	((PurpleApp*)be_app)->SendMessage(account, sent);
}


static void
signal_sent_im_msg(PurpleAccount* account, const char* receiver,
	const char* message)
{
	BMessage sent(IM_MESSAGE);
	sent.AddInt32("im_what", IM_MESSAGE_SENT);
	sent.AddString("chat_id", receiver);
	sent.AddString("user_id", purple_account_get_username(account));
	sent.AddString("body", purple_unescape_text(message));
	((PurpleApp*)be_app)->SendMessage(account, sent);
}


static void
signal_chat_topic_changed(PurpleConversation* conv, const char* who,
	const char* topic)
{
	BMessage subject(IM_MESSAGE);
	subject.AddInt32("im_what", IM_ROOM_SUBJECT_SET);
	subject.AddString("chat_id", purple_conversation_get_name(conv));
	subject.AddString("subject", topic);
	PurpleAccount* account = purple_conversation_get_account(conv);
	((PurpleApp*)be_app)->SendMessage(account, subject);
}


static void
signal_chat_buddy_joined(PurpleConversation* conv, const char* name,
	PurpleConvChatBuddyFlags flags, gboolean new_arrival)
{
	BMessage joined(IM_MESSAGE);
	if (new_arrival)
		joined.AddInt32("im_what", IM_ROOM_PARTICIPANT_JOINED);
	else {
		joined.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
		if (is_own_user(purple_conversation_get_account(conv), name) == true)
			return;
	}

	joined.AddString("chat_id", purple_conversation_get_name(conv));
	joined.AddString("user_id", name);

	PurpleAccount* account = purple_conversation_get_account(conv);
	((PurpleApp*)be_app)->SendMessage(account, joined);

	send_user_role(conv, name, flags);
}


static void
signal_chat_buddy_left(PurpleConversation* conv, const char* name,
	const char* reason)
{
	BMessage left(IM_MESSAGE);
	left.AddInt32("im_what", IM_ROOM_PARTICIPANT_LEFT);
	left.AddString("chat_id", purple_conversation_get_name(conv));
	left.AddString("user_id", name);
	left.AddString("body", reason);
	PurpleAccount* account = purple_conversation_get_account(conv);
	((PurpleApp*)be_app)->SendMessage(account, left);
}


// inviter == user_id, not user_name
static void
signal_chat_invited(PurpleAccount* account, const char* inviter,
	const char* chat, const char* message, const GHashTable* components)
{
	PurpleApp* app = (PurpleApp*)be_app;
	BString key(purple_account_get_username(account));
	key = key.Append("-invite-").Append(chat);

	GHashTable* data = (GHashTable*)components;
	app->fInviteList.AddItem(key, data);

	BMessage invited(IM_MESSAGE);
	invited.AddInt32("im_what", IM_ROOM_INVITE_RECEIVED);
	invited.AddString("chat_id", chat);
	invited.AddString("user_id", inviter);
	invited.AddString("body", purple_unescape_text(message));
	app->SendMessage(account, invited);
}


static void
signal_chat_buddy_flags(PurpleConversation* conv, const char* name,
	PurpleConvChatBuddyFlags oldflags, PurpleConvChatBuddyFlags newflags)
{
	send_user_role(conv, name, newflags);
}


static guint
ui_op_input_add(gint fd, PurpleInputCondition condition,
	PurpleInputFunction function, gpointer data)
{
	PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	GIOChannel *channel;
	GIOCondition cond = (GIOCondition)0;

	closure->function = function;
	closure->data = data;

	channel = g_io_channel_unix_new(fd);
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
					      _purple_glib_io_invoke, closure, g_free);

	g_io_channel_unref(channel);
	return closure->result;
}


static void
ui_op_disconnected(PurpleConnection* conn)
{
	PurpleAccount* account = purple_connection_get_account(conn);
	const PurpleConnectionErrorInfo* err
		= purple_account_get_current_error(account);
	if (!err) {
		BMessage status(IM_MESSAGE);
		status.AddInt32("im_what", IM_OWN_STATUS_SET);
		status.AddInt32("status", (int32)STATUS_OFFLINE);
		((PurpleApp*)be_app)->SendMessage(account, status);
	}
}


static void
ui_op_report_disconnect_reason(PurpleConnection* conn,
	PurpleConnectionError reason, const char* text)
{
	PurpleAccount* account = purple_connection_get_account(conn);
	PurpleStatus* status = purple_account_get_active_status(account);

	if (purple_connection_error_is_fatal(reason) == false)
		if (purple_status_is_online(status))
			purple_account_connect(account);
		else
			((PurpleApp*)be_app)->SendMessage(account,
				BMessage(PURPLE_SHUTDOWN_ADDON));
	else  {
		BMessage disabled(PURPLE_SHUTDOWN_ADDON);
		((PurpleApp*)be_app)->SendMessage(account, disabled);
	}
}


static void
ui_op_chat_rename_user(PurpleConversation* conv, const char* old_name,
	const char* new_name, const char* new_alias)
{
	PurpleAccount* account = purple_conversation_get_account(conv);
	PurpleApp* app = (PurpleApp*)be_app;

	if (is_own_user(account, old_name) == true) {
		const char* username = purple_account_get_username(account);
		app->fUserNicks.RemoveItemFor(username);
		app->fUserNicks.AddItem(username, new_name);
		purple_account_set_alias(account, new_name);
		return;
	}

	BString text = B_TRANSLATE("User changed name to %nick%");
	text.ReplaceAll("%nick%", new_name);

	BMessage left(IM_MESSAGE);
	left.AddInt32("im_what", IM_ROOM_PARTICIPANT_LEFT);
	left.AddString("user_id", old_name);
	left.AddString("chat_id", purple_conversation_get_name(conv));
	left.AddString("body", text);
	app->SendMessage(account, left);

	BMessage joined(IM_MESSAGE);
	joined.AddInt32("im_what", IM_ROOM_PARTICIPANT_JOINED);
	joined.AddString("user_id", new_name);
	joined.AddString("chat_id", purple_conversation_get_name(conv));
	app->SendMessage(account, joined);
}


static void
ui_op_add_room(PurpleRoomlist* list, PurpleRoomlistRoom* room)
{
	if (purple_roomlist_room_get_type(room) == PURPLE_ROOMLIST_ROOMTYPE_CATEGORY)
	{
		purple_roomlist_expand_category(list, room);
		return;
	}
	const char* name = purple_roomlist_room_get_name(room);
	if (name == NULL || BString(name).IsEmpty() == true)
		return;

	PurpleApp* app = (PurpleApp*)be_app;
	PurpleAccount* account = list->account;
	app->fRoomlists.AddItem(account, list);

	BMessage dirMsg(IM_MESSAGE);
	dirMsg.AddInt32("im_what", IM_ROOM_DIRECTORY);
	dirMsg.AddString("chat_id", purple_roomlist_room_get_name(room));
	dirMsg.AddBool("fromRoomlist", true);

	// Add relevant fields to message
	GList* roomIter, *listIter;
	for (listIter = purple_roomlist_get_fields(list),
				roomIter = purple_roomlist_room_get_fields(room);
			roomIter && listIter;
			roomIter = roomIter->next, listIter = listIter->next)
	{
		PurpleRoomlistField* listField = (PurpleRoomlistField*)listIter->data;
		if (listField == NULL || purple_roomlist_field_get_hidden(listField))
			continue;

		const char* label = purple_roomlist_field_get_label(listField);
		switch (purple_roomlist_field_get_type(listField)) {
			case PURPLE_ROOMLIST_FIELD_INT:
			{
				int32 num = GPOINTER_TO_INT(roomIter->data);
				if (strcmp(label, "Users") == 0)
					dirMsg.AddInt32("user_count", num);
				else
					dirMsg.AddInt32(label, num);
				break;
			}
			case PURPLE_ROOMLIST_FIELD_STRING:
			{
				const char* str = (const char*)roomIter->data;
				if (strcmp(label, "Topic") == 0)
					dirMsg.AddString("subject", str);
				else
					dirMsg.AddString(label, str);
				break;
			}
		}
	}

	// Find the room's categor[y|ies], if any
	BString category;
	PurpleRoomlistRoom* parent = room;
	while ((parent = purple_roomlist_room_get_parent(parent)) != NULL) {
		const char* name = purple_roomlist_room_get_name(parent);
		if (name == NULL || strcmp(name, "true") == 0
				|| purple_roomlist_room_get_type(parent)
					!= PURPLE_ROOMLIST_ROOMTYPE_CATEGORY)
			continue;

		if (category.IsEmpty() == false) {
			category.Prepend("→");
			category.Prepend(name);
		}
		else
			category << name;
	}
	if (category.IsEmpty() == false)
		dirMsg.AddString("category", category);

	app->SendMessage(account, dirMsg);
}


static void*
ui_op_request_input(const char* title, const char* primary,
	const char* secondary, const char* default_value, gboolean multiline,
	gboolean masked, gchar* hint, const char* ok_text, GCallback ok_cb,
	const char* cancel_text, GCallback cancel_cb, PurpleAccount* account,
	const char* who, PurpleConversation* conv, void* user_data)
{
	std::cerr << "request input: " << title << std::endl;
	return NULL;
}


static void*
ui_op_request_choice(const char* title, const char* primary,
	const char* secondary, int default_value, const char* ok_text,
	GCallback ok_cb, const char* cancel_text, GCallback cancel_cb,
	PurpleAccount* account, const char* who, PurpleConversation* conv,
	void* user_data, va_list choices)
{
	std::cerr << "request choice: " << title << std::endl;
	return NULL;
}


static void*
ui_op_request_action(const char* title, const char* primary,
	const char* secondary, int default_action, PurpleAccount* account,
	const char* who, PurpleConversation* conv, void* user_data,
	size_t action_count, va_list actions)
{
	PurpleDialog* win =
		new PurpleDialog(title, primary, secondary, account, actions,
			action_count, user_data);
	win->Show();
	return NULL;
}


static void*
ui_op_request_fields(const char* title, const char* primary,
	const char* secondary, PurpleRequestFields* fields, const char* ok_text,
	GCallback ok_cb, const char* cancel_text, GCallback cancel_cb,
	PurpleAccount* account, const char* who, PurpleConversation* conv,
	void* user_data)
{
	std::cerr << "request fields from " << purple_account_get_username(account)
		<< ": " << primary << std::endl;
	return NULL;
}


static void*
ui_op_request_file(const char* title, const char* filename, gboolean savedialog,
	GCallback ok_cb, GCallback cancel_cb, PurpleAccount* account,
	const char* who, PurpleConversation* conv, void* user_data)
{
	std::cerr << "request file: " << title << std::endl;
	return NULL;
}


static void*
ui_op_request_folder(const char* title, const char* dirname, GCallback ok_cb,
	GCallback cancel_cb, PurpleAccount* account, const char* who,
	PurpleConversation* conv, void* user_data)
{
	std::cerr << "request folder: " << title << std::endl;
	return NULL;
}


static void*
ui_op_request_action_with_icon(const char* title, const char* primary,
	const char* secondary, int default_action, PurpleAccount* account,
	const char* who, PurpleConversation* conv, gconstpointer icon_data,
	gsize icon_size, void* user_data, size_t action_count, va_list actions)
{
	std::cerr << "request action with icon: " << title << std::endl;
	return NULL;
}


static void*
ui_op_notify_message(PurpleNotifyMsgType type, const char* title,
	const char* primary, const char* secondary)
{
	BString text = _tr(primary);
	text << "\n" << _tr(secondary);

	BAlert* alert = new BAlert(title, text.String(), B_TRANSLATE("OK"));

	if (type == PURPLE_NOTIFY_MSG_WARNING)
		alert->SetType(B_WARNING_ALERT);
	else if (type == PURPLE_NOTIFY_MSG_ERROR)
		alert->SetType(B_STOP_ALERT);

	alert->Go(NULL);
	return NULL;
}


static void
callback_set_public_alias_failure(PurpleAccount* account, const char* error)
{
	BString text = B_TRANSLATE("Couldn't set your nick:\n%error%");
	text.ReplaceAll("%error%", error);

	BAlert* alert = new BAlert(B_TRANSLATE("Failed to set nickname"), text,
		B_TRANSLATE("OK"));
	alert->Go(NULL);
}


bool
is_own_user(PurpleAccount* account, const char* name)
{
	PurpleApp* app = ((PurpleApp*)be_app);
	BString username = purple_account_get_username(account);
	BString display = purple_account_get_name_for_display(account);

	if (app->fUserNicks.ValueFor(username) != display) {
		app->fUserNicks.RemoveItemFor(username);
		app->fUserNicks.AddItem(username, display);
		send_own_info(account);
	}

	if (name == username || name == display)
		return true;
	return false;
}


void
load_account_buddies(PurpleAccount* account)
{
	BDirectory dir(buddies_cache(account));

	entry_ref ref;
	while (dir.GetNextRef(&ref) == B_OK) {
		BString user_id, user_name;

		BFile file(&ref, B_READ_ONLY);
		file.ReadAttrString("purple:user_id", &user_id);
		file.ReadAttrString("purple:alias", &user_name);

		if (user_id.IsEmpty() == false) {
			PurpleBuddy* buddy =
				purple_buddy_new(account, user_id.String(), user_name.String());
			purple_blist_add_buddy(buddy, NULL, NULL, NULL);
			purple_account_add_buddy_with_invite(account, buddy, NULL);
		}
	}
}


void
send_own_info(PurpleAccount* account)
{
	BMessage info(IM_MESSAGE);
	info.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	info.AddString("user_id", purple_account_get_username(account));
	info.AddString("user_name", purple_account_get_name_for_display(account));
	((PurpleApp*)be_app)->SendMessage(account, info);
}


void
send_user_role(PurpleConversation* conv, const char* name,
	PurpleConvChatBuddyFlags flags)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp ― User roles"

	if (flags == 0) return;

	BString role_title;
	int32 role_perms = 0 | PERM_READ | PERM_WRITE;
	int32 role_priority = 0;

	if (flags & PURPLE_CBFLAGS_FOUNDER) {
		role_title = B_TRANSLATE("Founder");
		role_priority = 3;
	}
	if (flags & PURPLE_CBFLAGS_OP) {
		if (role_title.IsEmpty() == true)
			role_title = B_TRANSLATE("Operator");
		role_perms |= PERM_ROOM_SUBJECT | PERM_ROOM_NAME;
		role_priority = 2;
	}
	if (flags & PURPLE_CBFLAGS_HALFOP) {
		if (role_title.IsEmpty() == true)
			role_title = B_TRANSLATE("Moderator");
		role_perms |= PERM_ROOM_SUBJECT | PERM_ROOM_NAME;
		role_priority = 3;
	}

	if (role_title.IsEmpty() == true) return;

	BMessage role(IM_MESSAGE);
	role.AddInt32("im_what", IM_ROOM_ROLECHANGED);
	role.AddString("user_id", name);
	role.AddString("chat_id", purple_conversation_get_name(conv));
	role.AddString("role_title", role_title);
	role.AddInt32("role_perms", role_perms);
	role.AddInt32("role_priority", role_priority);
	((PurpleApp*)be_app)->SendMessage(purple_conversation_get_account(conv),
		role);
}


PurpleStatusPrimitive
cardie_status_to_purple(UserStatus status)
{
	PurpleStatusPrimitive type = PURPLE_STATUS_UNSET;
	switch (status)
	{
		case STATUS_ONLINE:
			type = PURPLE_STATUS_AVAILABLE;
			break;
		case STATUS_AWAY:
			type = PURPLE_STATUS_AWAY;
			break;
		case STATUS_DO_NOT_DISTURB:
			type = PURPLE_STATUS_UNAVAILABLE;
			break;
		case STATUS_CUSTOM_STATUS:
			type = PURPLE_STATUS_AVAILABLE;
			break;
		case STATUS_INVISIBLE:
			type = PURPLE_STATUS_INVISIBLE;
			break;
		case STATUS_OFFLINE:
			type = PURPLE_STATUS_OFFLINE;
			break;
	}

	return type;
}


UserStatus
purple_status_to_cardie(PurpleStatus* status)
{
	PurpleStatusPrimitive prim =
		purple_status_type_get_primitive(purple_status_get_type(status));

	switch (prim)
	{
		case PURPLE_STATUS_AWAY:
			return STATUS_AWAY;
		case PURPLE_STATUS_UNAVAILABLE:
			return STATUS_DO_NOT_DISTURB;
			break;
		case PURPLE_STATUS_INVISIBLE:
			return STATUS_INVISIBLE;
			break;
		case PURPLE_STATUS_OFFLINE:
			return STATUS_OFFLINE;
	}
	return STATUS_ONLINE;
}


const char*
purple_connection_error_name(const PurpleConnectionErrorInfo* error)
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleApp ― Connection errors"

	switch (error->type)
	{
		case PURPLE_CONNECTION_ERROR_NETWORK_ERROR:
			return B_TRANSLATE("Network error");
		case PURPLE_CONNECTION_ERROR_INVALID_USERNAME:
			return B_TRANSLATE("Invalid username");
		case PURPLE_CONNECTION_ERROR_AUTHENTICATION_FAILED:
			return B_TRANSLATE("Authentication failed");
		case PURPLE_CONNECTION_ERROR_AUTHENTICATION_IMPOSSIBLE:
			return B_TRANSLATE("Authentication impossible");
		case PURPLE_CONNECTION_ERROR_NO_SSL_SUPPORT:
			return B_TRANSLATE("SSL unsupported");
		case PURPLE_CONNECTION_ERROR_ENCRYPTION_ERROR:
			return B_TRANSLATE("Encryption error");
		case PURPLE_CONNECTION_ERROR_NAME_IN_USE:
			return B_TRANSLATE("Username in use");
		case PURPLE_CONNECTION_ERROR_INVALID_SETTINGS:
			return B_TRANSLATE("Settings invalid");
		case PURPLE_CONNECTION_ERROR_CERT_NOT_PROVIDED:
			return B_TRANSLATE("No SSL certificate provided");
		case PURPLE_CONNECTION_ERROR_CERT_UNTRUSTED:
			return B_TRANSLATE("Untrusted SSL certificate");
		case PURPLE_CONNECTION_ERROR_CERT_EXPIRED:
			return B_TRANSLATE("Expired SSL certificate");
		case PURPLE_CONNECTION_ERROR_CERT_NOT_ACTIVATED:
			return B_TRANSLATE("Unactivated SSL certificate");
		case PURPLE_CONNECTION_ERROR_CERT_HOSTNAME_MISMATCH:
			return B_TRANSLATE("Certificate and hostname conflict");
		case PURPLE_CONNECTION_ERROR_CERT_FINGERPRINT_MISMATCH:
			return B_TRANSLATE("Certifcate and fingerprint conflict");
		case PURPLE_CONNECTION_ERROR_CERT_SELF_SIGNED:
			return B_TRANSLATE("Self-signed certificate");
		case PURPLE_CONNECTION_ERROR_CERT_OTHER_ERROR:
			return B_TRANSLATE("Certificate error");
	}
	return B_TRANSLATE("Connection error");
}


const char*
purple_cache()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;
	path.Append(APP_NAME "/Cache/Add-Ons/" PURPLE_ADDON);
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
account_cache(PurpleAccount* account)
{
	const char* purple_user = purple_account_get_username(account);
	const char* cardie_user = NULL;

	StringMap usernames = ((PurpleApp*)be_app)->fAccounts;
	for (int i = 0; i < usernames.CountItems(); i++)
		if (usernames.ValueAt(i) == purple_user) {
			cardie_user = usernames.KeyAt(i);
			break;
		}
	if (cardie_user == NULL)
		return NULL;

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;
	path.Append(APP_NAME "/Cache/Accounts/");
	path.Append(cardie_user);

	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
buddies_cache(PurpleAccount* account)
{
	BPath path(account_cache(account));
	path.Append("Contacts");
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	return path.Path();
}


const char*
buddy_cache(PurpleBuddy* buddy)
{
	BPath path(buddies_cache(purple_buddy_get_account(buddy)));
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;
	path.Append(purple_buddy_get_name(buddy));
	return path.Path();
}


void
purple_plugins_add_finddir(directory_which finddir)
{
	BPath path;
	if (find_directory(finddir, &path) == B_OK) {
		path.Append("purple-2");
		purple_plugins_add_search_path(path.Path());
	}
}


void
update_buddy(const char* path, BString user_id, BString user_name)
{
	BFile file(path, B_WRITE_ONLY | B_CREATE_FILE);
	if (file.InitCheck() != B_OK) {
		std::cerr << "Failed to update buddy at " << path << std::endl;
		return;
	}
	file.WriteAttrString("purple:user_id", &user_id);
	if (user_name.IsEmpty() == false)
		file.WriteAttrString("purple:alias", &user_name);
}


static gboolean
_purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
	PurpleGLibIOClosure *closure = (PurpleGLibIOClosure*)data;
	PurpleInputCondition purple_cond = (PurpleInputCondition)0;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  purple_cond);

	return TRUE;
}


const char*
_tr(const char* string)
{
	return BString(dgettext("pidgin", string)).ReplaceFirst("_", "").String();
}
