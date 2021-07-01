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

#ifndef _PURPLE_APP_H
#define _PURPLE_APP_H

#include <libpurple/purple.h>

#include <Application.h>
#include <FindDirectory.h>
#include <ObjectList.h>
#include <StringList.h>

#include <libsupport/KeyMap.h>
#include <AppConstants.h>


typedef KeyMap<BString, BString> Accounts; // Cardie username → Purple username
typedef KeyMap<BString, thread_id> AccountThreads; // Purple username → Thread

const uint32 G_MAIN_LOOP = 'GLml';
const uint32 CHECK_APP = 'Paca';


#define PURPLE_GLIB_READ_COND  (G_IO_IN | G_IO_HUP | G_IO_ERR)
#define PURPLE_GLIB_WRITE_COND (G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL)
#define PURPLE_UI_ID "cardie"


typedef struct _PurpleGLibIOClosure {
	PurpleInputFunction function;
	guint result;
	gpointer data;
} PurpleGLibIOClosure;

struct _PurpleStatus
{
	PurpleStatusType *type;
	PurplePresence *presence;

	gboolean active;
	GHashTable *attr_values;
};

typedef struct _ProtocolInfo {
	BString name;
	BString id;
	BMessage settingsTemplate;
} ProtocolInfo;


int main(int argc, char** argv);


class PurpleApp : public BApplication {
public:
						PurpleApp();

	virtual	void		MessageReceived(BMessage* msg);
			void		ImMessage(BMessage* msg);

			void		SendMessage(thread_id thread, BMessage msg);
			void		SendMessage(PurpleAccount* account, BMessage msg);

private:
			void		_GetProtocolsInfo();
			void		_SaveProtocolInfo(PurplePlugin* plugin);

			BMessage	_ParseProtoOptions(PurplePluginProtocolInfo* info);
			void		_ParseCardieSettings(BMessage* settings);

		PurplePlugin*	_PluginFromMessage(BMessage* msg);
		PurpleAccount*	_AccountFromMessage(BMessage* msg);
	PurpleConversation*	_ConversationFromMessage(BMessage* msg);

	Accounts fAccounts;
	AccountThreads fAccountThreads;
	BObjectList<ProtocolInfo> fProtocols;

	GMainLoop* fGloop;
};


			status_t	init_libpurple();
			void		init_ui_ops();
			void		init_signals();

// Connection signals
	 static void		signal_connection_error(PurpleConnection* gc,
							PurpleConnectionError err, const gchar* desc);

// Account signals
	 static void		signal_account_signed_on(PurpleAccount* account);
	 static void		signal_account_status_changed(PurpleAccount* account,
							PurpleStatus* old, PurpleStatus* cur);

// Buddy-list signals
	 static void		signal_blist_node_added(PurpleBlistNode* node);
	 static void		signal_blist_node_removed(PurpleBlistNode* node);

// Conversation signals
	 static void		signal_chat_joined(PurpleConversation* conv);
	 static void		signal_chat_left(PurpleConversation* conv);
	 static void		signal_received_chat_msg(PurpleAccount* account,
	 						char* sender, char* message,
							PurpleConversation* conv, PurpleMessageFlags flags);
	 static void		signal_sent_chat_msg(PurpleAccount* account,
							const char* message, int conv_id);
	 static void		signal_sent_im_msg(PurpleAccount* account,
							const char* receiver, const char* message);
	 static void		signal_chat_buddy_joined(PurpleConversation* conv,
							const char* name, PurpleConvChatBuddyFlags flags,
							gboolean new_arrival);

// Util
PurpleStatusPrimitive	cardie_status_to_purple(UserStatus status);
		UserStatus		purple_status_to_cardie(PurpleStatus* status);

		const char*		purple_cache();
			void		purple_plugins_add_finddir(directory_which finddir);

	static guint		_purple_glib_input_add(gint fd,
							PurpleInputCondition condition,
							PurpleInputFunction function, gpointer data);
	static gboolean		_purple_glib_io_invoke(GIOChannel *source,
							GIOCondition condition, gpointer data);

#endif // _PURPLE_APP_H
