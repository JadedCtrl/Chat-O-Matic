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


int
main(int arc, char** argv)
{
	PurpleApp app;
	app.Run();
	return 0;
}


PurpleApp::PurpleApp()
	:
	BApplication("application/x-vnd.cardie.purple")
{
	if (init_libpurple() != B_OK)
		std::cerr << "libpurple initialization failed. Please report!\n";

	_GetProtocolsInfo();
}


void
PurpleApp::MessageReceived(BMessage* msg)
{
	int64 thread_id;
	if (msg->FindInt64("thread_id", &thread_id) != B_OK)
		return;

	switch (msg->what)
	{
		case PURPLE_REQUEST_PROTOCOL_COUNT:
		{
			send_data(thread_id, fProtocols.CountItems(), NULL, 0);
			break;
		}
		case PURPLE_REQUEST_PROTOCOL_INFO:
		{
			int32 index = msg->FindInt32("index", 0);
			ProtocolInfo* info = fProtocols.ItemAt(index);

			BMessage protocolInfo = info->settingsTemplate;
			protocolInfo.AddString("name", info->name);
			protocolInfo.AddString("id", info->id);

			// Send message to requester
			ssize_t size = protocolInfo.FlattenedSize();
			char buffer[size];

			send_data(thread_id, size, NULL, 0);
			protocolInfo.Flatten(buffer, size);
			send_data(thread_id, 0, buffer, size);
			break;
		}
		default:
			BApplication::MessageReceived(msg);
	}
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

	GList* prefIter = info->protocol_options;
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
			case PURPLE_PREF_STRING_LIST: {
				bType = B_STRING_TYPE;
				BString implicit;
				GList* lists;
				for (int j = 0; lists != NULL; lists = lists->next)
					implicit << " " << lists->data;
				setting.AddString("default", implicit);
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
	purple_eventloop_set_ui_ops(&_glib_eventloops);

	if (!purple_core_init("cardie"))
		return B_ERROR;
	return B_OK;
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
