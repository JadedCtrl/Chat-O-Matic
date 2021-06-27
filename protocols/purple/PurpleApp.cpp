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

#include <MessageRunner.h>

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
	fGRunner = new BMessageRunner(this, new BMessage(G_MAIN_LOOP), 100000, -1);
}


void
PurpleApp::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
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
			int32 index = msg->FindInt32("index", 0);
			ProtocolInfo* info = fProtocols.ItemAt(index);

			BMessage protocolInfo = info->settingsTemplate;
			protocolInfo.AddString("name", info->name);
			protocolInfo.AddString("id", info->id);
			SendMessage(thread_id, protocolInfo);

			break;
		}
		case PURPLE_LOAD_ACCOUNT:
		{
			_ParseCardieSettings(msg);
			break;
		}
		case PURPLE_REGISTER_THREAD:
		{
			msg->PrintToStream();
			BString accName = msg->FindString("account_name");
			BString username = fAccounts.ValueFor(accName);
			int64 thread;
			if (username.IsEmpty() == true
					|| msg->FindInt64("thread_id", &thread) != B_OK)
				break;
			fAccountThreads.AddItem(username, thread);
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

	// Add any UserSplits (that is, parts of the protocols 'username' format)
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
	purple_signal_connect(purple_connections_get_handle(), "signed-on", &handle,
		PURPLE_CALLBACK(signal_signed_on), NULL);
	purple_signal_connect(purple_connections_get_handle(), "connection-error", &handle,
		PURPLE_CALLBACK(signal_connection_error), NULL);
}


static void
signal_signed_on(PurpleConnection* gc)
{
	BMessage readyMsg(IM_MESSAGE);
	readyMsg.AddInt32("im_what", IM_PROTOCOL_READY);

	PurpleApp* app = (PurpleApp*)be_app;
	app->SendMessage(purple_connection_get_account(gc), readyMsg);
}


static void
signal_connection_error(PurpleConnection* gc, PurpleConnectionError err,
	const gchar* desc)
{
	std::cout << "Connection failed: " << (const char*)desc << std::endl;
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
