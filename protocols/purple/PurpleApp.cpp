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
#include <libpurple/purple.h>
#include <libpurple/status.h>

#include <Directory.h>
#include <MessageRunner.h>
#include <Path.h>
#include <Roster.h>

#include <Cardie.h>
#include <ChatProtocolMessages.h>

#include "Purple.h"
#include "PurpleMessages.h"


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

			BMessage protocolInfo = info->settingsTemplate;
			protocolInfo.AddString("name", info->name);
			protocolInfo.AddString("id", info->id);
			SendMessage(thread_id, protocolInfo);
			break;
		}
		case PURPLE_CONNECT_ACCOUNT:
		{
			_ParseCardieSettings(msg);
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
			break;
		}
		case PURPLE_REQUEST_DISCONNECT:
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
	switch (msg->FindInt32("im_what"))
	{
		case IM_SET_OWN_STATUS:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			UserStatus status = (UserStatus)msg->FindInt32("status");
			PurpleStatusPrimitive prim = cardie_status_to_purple(status);
			const char* primId = purple_primitive_get_id_from_type(prim);

			std::cout << "setting status to " << primId << "…\n";
			purple_account_set_status(account, primId, true);
			break;
		}
		case IM_SEND_MESSAGE:
		{
			BString body;
			if (msg->FindString("body", &body) != B_OK)	return;

			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			PurpleConvIm* im = purple_conversation_get_im_data(conv);

			if (chat != NULL)
				purple_conv_chat_send(chat, body.String());
			else if (im != NULL)
				purple_conv_im_send(im, body.String());
			break;
		}
		case IM_JOIN_ROOM:
		{
			PurpleAccount* account = _AccountFromMessage(msg);
			PurpleConnection* conn =  purple_account_get_connection(account);
			BString chat_id = msg->FindString("chat_id");
			if (account == NULL || conn == NULL || chat_id.IsEmpty() == true)
				break;

			PurplePluginProtocolInfo* info =
				PURPLE_PLUGIN_PROTOCOL_INFO(purple_connection_get_prpl(conn));

			if (info->chat_info_defaults != NULL) {
				GHashTable* hash = info->chat_info_defaults(conn,
					chat_id.String());
				serv_join_chat(conn, hash);
			}
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
			if (chat != NULL)
				meta.AddString("subject", purple_conv_chat_get_topic(chat));
			SendMessage(purple_conversation_get_account(conv), meta);
			break;
		}
		case IM_GET_ROOM_PARTICIPANTS:
		{
			PurpleConversation* conv = _ConversationFromMessage(msg);
			PurpleConvChat* chat = purple_conversation_get_chat_data(conv);
			PurpleConvIm* im = purple_conversation_get_im_data(conv);
			if (chat == NULL && im == NULL) return;

			BStringList user_ids;
			if (im != NULL)
				user_ids.Add(BString(purple_conversation_get_name(conv)));
			else {
				GList* users = purple_conv_chat_get_users(chat);
				for (int i = 0; users != NULL; users = users->next) {
					PurpleConvChatBuddy* user = (PurpleConvChatBuddy*)users->data;
					user_ids.Add(BString(purple_conv_chat_cb_get_name(user)));
				}
			}

			BMessage parts(IM_MESSAGE);
			parts.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
			parts.AddString("chat_id", purple_conversation_get_name(conv));
			parts.AddStrings("user_id", user_ids);
			SendMessage(purple_conversation_get_account(conv), parts);
			break;
		}
		case IM_GET_CONTACT_LIST:
		{
			BStringList user_ids;
			GSList* buddies = purple_blist_get_buddies();
			for (int i = 0; buddies != NULL; buddies = buddies->next) {
				PurpleBuddy* buddy = (PurpleBuddy*)buddies->data;
				user_ids.Add(BString(purple_buddy_get_name(buddy)));
			}

			BMessage roster(IM_MESSAGE);
			roster.AddInt32("im_what", IM_CONTACT_LIST);
			roster.AddStrings("user_id", user_ids);
			SendMessage(_AccountFromMessage(msg), roster);
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
	proto->id = plugin->info->id;
	proto->name = plugin->info->name;

	PurplePluginProtocolInfo* info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin);
	proto->settingsTemplate = _ParseProtoOptions(info);

	fProtocols.AddItem(proto);
}


BMessage
PurpleApp::_ParseProtoOptions(PurplePluginProtocolInfo* info)
{
	BMessage temp;

	// Add a "username" setting, if not explicitly specified
	GList* prefIter = info->protocol_options;
	for (int i = 0; prefIter != NULL; prefIter = prefIter->next) {
		PurpleAccountOption* pref = (PurpleAccountOption*)prefIter->data;

		if (pref->pref_name == BString("username"))
			break;
		else if (prefIter->next == NULL) {
			BMessage setting;
			setting.AddString("name", "username");
			setting.AddString("description", "Username");
			setting.AddString("error", "A username needs to be specified!");
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
		setting.AddString("description", split->text);
		setting.AddString("default", split->default_value);
		setting.AddInt32("type", B_STRING_TYPE);
		temp.AddMessage("setting", &setting);
	}

	// Password setting
	BMessage passwd;
	passwd.AddString("name", "password");
	passwd.AddString("description", "Password");
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
		setting.AddString("description", pref->text);

		switch (type)
		{
			case PURPLE_PREF_BOOLEAN:
			{
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
		setting.AddString("error",
			BString(pref->text).Append(" needs to be specified."));
		setting.AddInt32("type", bType);
		temp.AddMessage("setting", &setting);
	}
	return temp;
}


void
PurpleApp::_ParseCardieSettings(BMessage* settings)
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
}


PurplePlugin*
PurpleApp::_PluginFromMessage(BMessage* msg)
{
	return purple_plugins_find_with_id(msg->FindString("protocol"));
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


static PurpleEventLoopUiOps _glib_eventloops =
{
	g_timeout_add,
	g_source_remove,
	_purple_glib_input_add,
	g_source_remove,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


status_t
init_libpurple()
{
	init_ui_ops();

	purple_util_set_user_dir(purple_cache());

	BString cachePlugin = BString(purple_cache()).Append("/plugins/");
	purple_plugins_add_search_path(cachePlugin.String());
	purple_plugins_add_finddir(B_USER_LIB_DIRECTORY);
	purple_plugins_add_finddir(B_SYSTEM_LIB_DIRECTORY);
	purple_plugins_add_finddir(B_USER_NONPACKAGED_LIB_DIRECTORY);
	purple_plugins_add_finddir(B_SYSTEM_NONPACKAGED_LIB_DIRECTORY);

	if (!purple_core_init(PURPLE_UI_ID))
		return B_ERROR;

	purple_set_blist(purple_blist_new());
	purple_blist_load();

	init_signals();
	return B_OK;
}


void
init_ui_ops()
{
	purple_eventloop_set_ui_ops(&_glib_eventloops);
}


void
init_signals()
{
	int handle;

	purple_signal_connect(purple_connections_get_handle(), "connection-error",
		&handle, PURPLE_CALLBACK(signal_connection_error), NULL);

	purple_signal_connect(purple_accounts_get_handle(), "account-signed-on",
		&handle, PURPLE_CALLBACK(signal_account_signed_on), NULL);
	purple_signal_connect(purple_accounts_get_handle(), "account-status-changed",
		&handle, PURPLE_CALLBACK(signal_account_status_changed), NULL);

	purple_signal_connect(purple_blist_get_handle(), "blist-node-added",
		&handle, PURPLE_CALLBACK(signal_blist_node_added), NULL);
	purple_signal_connect(purple_blist_get_handle(), "blist-node-removed",
		&handle, PURPLE_CALLBACK(signal_blist_node_removed), NULL);

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
	purple_signal_connect(purple_conversations_get_handle(), "chat-buddy-joined",
		&handle, PURPLE_CALLBACK(signal_chat_buddy_joined), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "chat-invited",
		&handle, PURPLE_CALLBACK(signal_chat_invited), NULL);
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

	BMessage info(IM_MESSAGE);
	info.AddInt32("im_what", IM_OWN_CONTACT_INFO);
	info.AddString("user_id", purple_account_get_username(account));
	info.AddString("user_name", purple_account_get_name_for_display(account));
	((PurpleApp*)be_app)->SendMessage(account, info);
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
	add.AddInt32("im_what", IM_CONTACT_LIST);
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
	rem.AddInt32("im_what", IM_CONTACT_LIST_CONTACT_REMOVED);
	rem.AddString("user_id", purple_buddy_get_name(buddy));
	((PurpleApp*)be_app)->SendMessage(purple_buddy_get_account(buddy), rem);
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
	BString chat_id = BString(purple_conversation_get_name(conv));
	if (chat_id.IsEmpty() == true)
		chat_id = sender;

	BMessage chat(IM_MESSAGE);
	chat.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	chat.AddString("chat_id", chat_id);
	chat.AddString("user_id", sender);
	chat.AddString("body", message);
	((PurpleApp*)be_app)->SendMessage(account, chat);
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
	sent.AddString("body", message);
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
	sent.AddString("body", message);
	((PurpleApp*)be_app)->SendMessage(account, sent);
}


static void
signal_chat_buddy_joined(PurpleConversation* conv, const char* name,
	PurpleConvChatBuddyFlags flags, gboolean new_arrival)
{
	BMessage joined(IM_MESSAGE);
	if (new_arrival)
		joined.AddInt32("im_what", IM_ROOM_PARTICIPANT_JOINED);
	else
		joined.AddInt32("im_what", IM_ROOM_PARTICIPANTS);
	joined.AddString("chat_id", purple_conversation_get_name(conv));
	joined.AddString("user_id", name);
	PurpleAccount* account = purple_conversation_get_account(conv);
	((PurpleApp*)be_app)->SendMessage(account, joined);
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
	invited.AddString("body", message);
	app->SendMessage(account, invited);
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


void
purple_plugins_add_finddir(directory_which finddir)
{
	BPath path;
	if (find_directory(finddir, &path) == B_OK) {
		path.Append("purple-2");
		purple_plugins_add_search_path(path.Path());
	}
}


static guint _purple_glib_input_add(gint fd, PurpleInputCondition condition,
	PurpleInputFunction function, gpointer data)
{
	PurpleGLibIOClosure *closure = g_new0(PurpleGLibIOClosure, 1);
	GIOChannel *channel;
	GIOCondition cond = (GIOCondition)0;

	closure->function = function;
	closure->data = data;

//	if (condition & PURPLE_INPUT_READ)
//		cond |= PURPLE_GLIB_READ_COND;
//	if (condition & PURPLE_INPUT_WRITE)
//		cond |= PURPLE_GLIB_WRITE_COND;

	channel = g_io_channel_unix_new(fd);
	closure->result = g_io_add_watch_full(channel, G_PRIORITY_DEFAULT, cond,
					      _purple_glib_io_invoke, closure, g_free);

	g_io_channel_unref(channel);
	return closure->result;
}


static gboolean _purple_glib_io_invoke(GIOChannel *source, GIOCondition condition, gpointer data)
{
	PurpleGLibIOClosure *closure = (PurpleGLibIOClosure*)data;
	PurpleInputCondition purple_cond = (PurpleInputCondition)0;

//	if (condition & PURPLE_GLIB_READ_COND)
//		purple_cond |= PURPLE_INPUT_READ;
//	if (condition & PURPLE_GLIB_WRITE_COND)
//		purple_cond |= PURPLE_INPUT_WRITE;

	closure->function(closure->data, g_io_channel_unix_get_fd(source),
			  purple_cond);

	return TRUE;
}
