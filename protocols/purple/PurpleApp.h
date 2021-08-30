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
#include <UserStatus.h>


typedef KeyMap<BString, BString> StringMap;
typedef KeyMap<BString, thread_id> ThreadMap;
typedef KeyMap<BString, GHashTable*> HashMap;
typedef KeyMap<PurpleAccount*, PurpleRoomlist*> RoomMap;

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
	BMessage accountTemplate;
	BMessage roomTemplate;
	BString iconName;
} ProtocolInfo;


int main(int argc, char** argv);


class PurpleApp : public BApplication {
public:
						PurpleApp();

	virtual	void		MessageReceived(BMessage* msg);
			void		ImMessage(BMessage* msg);

			void		SendMessage(thread_id thread, BMessage msg);
			void		SendMessage(PurpleAccount* account, BMessage msg);

	HashMap fInviteList;
	StringMap fUserNicks; // Purple username → Nickname for Cardie
	StringMap fAccounts; // Cardie account name → Purple username
	RoomMap fRoomlists; // Purple account → Purple roomlist

private:
			void		_SendSysText(PurpleConversation* conv, const char* text);

			void		_GetProtocolsInfo();
			void		_SaveProtocolInfo(PurplePlugin* plugin);

			BMessage	_GetAccountTemplate(PurplePluginProtocolInfo* info);
			BMessage	_GetRoomTemplate(PurplePluginProtocolInfo* info);
			BMessage	_GetCommands(PurpleAccount* account);

			void		_ParseAccountTemplate(BMessage* settings);
			GHashTable*	_ParseRoomTemplate(BMessage* msg);
			GHashTable* _FindRoomlistComponents(BMessage* msg);

		PurplePlugin*	_PluginFromMessage(BMessage* msg);
	PurpleConnection*	_ConnectionFromMessage(BMessage* msg);
		PurpleAccount*	_AccountFromMessage(BMessage* msg);
	PurpleConversation*	_ConversationFromMessage(BMessage* msg);

	ThreadMap fAccountThreads; // Purple username → Thread
	BObjectList<ProtocolInfo> fProtocols;
	GMainLoop* fGloop;
};


			status_t	init_libpurple();
			void		init_gettext();
			void		init_ui_ops();
			void		init_signals();

// Connection signals
	 static void		signal_connection_error(PurpleConnection* gc,
							PurpleConnectionError err, const gchar* desc);

// Account signals
	 static void		signal_account_signed_on(PurpleAccount* account);
	 static void		signal_account_error_changed(PurpleAccount* account,
							const PurpleConnectionErrorInfo* old_error,
							const PurpleConnectionErrorInfo* current_error);
	 static void		signal_account_alias_changed(PurpleAccount* account,
							const char* old);
	 static void		signal_account_status_changed(PurpleAccount* account,
							PurpleStatus* old, PurpleStatus* cur);

// Buddy-list signals
	 static void		signal_blist_node_added(PurpleBlistNode* node);
	 static void		signal_blist_node_removed(PurpleBlistNode* node);
	 static void		signal_buddy_status_changed(PurpleBuddy* buddy,
							PurpleStatus* old_status, PurpleStatus* status);
	 static void		signal_buddy_icon_changed(PurpleBuddy* buddy);

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
	 static void		signal_chat_topic_changed(PurpleConversation* conv,
	 						const char* who, const char* topic);
	 static void		signal_chat_buddy_joined(PurpleConversation* conv,
							const char* name, PurpleConvChatBuddyFlags flags,
							gboolean new_arrival);
	 static void		signal_chat_buddy_left(PurpleConversation* conv,
							const char* name, const char* reason);
	 static void		signal_chat_invited(PurpleAccount* account,
							const char* inviter, const char* chat,
							const char* message, const GHashTable* components);
	 static void		signal_chat_buddy_flags(PurpleConversation* conv,
							const char* name, PurpleConvChatBuddyFlags oldflags,
							PurpleConvChatBuddyFlags newflags);

// EventLoop ui ops
	 static guint		ui_op_input_add(gint fd,
							PurpleInputCondition condition,
							PurpleInputFunction function, gpointer data);

// Connection ui ops
	 static void		ui_op_disconnected(PurpleConnection* conn);
	 static void		ui_op_report_disconnect_reason(PurpleConnection* conn,
							PurpleConnectionError reason, const char* text);

// Conversation ui ops
	 static void		ui_op_chat_rename_user(PurpleConversation* conv, 
							const char* old_name, const char* new_name,
							const char* new_alias);

// Roomlist ui ops
	 static void		ui_op_add_room(PurpleRoomlist* list,
							PurpleRoomlistRoom* room);

// Request ui ops
	 static void*		ui_op_request_input(const char* title,
							const char* primary, const char* secondary,
							const char* default_value, gboolean multiline,
							gboolean masked, gchar* hint, const char* ok_text,
							GCallback ok_cb, const char* cancel_text,
							GCallback cancel_cb, PurpleAccount* account,
							const char* who, PurpleConversation* conv,
							void* user_data);
	 static void*		ui_op_request_choice(const char* title,
							const char* primary, const char* secondary,
							int default_value, const char* ok_text,
							GCallback ok_cb, const char* cancel_text,
							GCallback cancel_cb, PurpleAccount* account,
							const char* who, PurpleConversation* conv,
							void* user_data, va_list choices);
	 static void*		ui_op_request_action(const char* title,
							const char* primary, const char* secondary,
							int default_action, PurpleAccount* account,
							const char* who, PurpleConversation* conv,
							void* user_data, size_t action_count,
							va_list actions);
	 static void*		ui_op_request_fields(const char* title,
							const char* primary, const char* secondary,
							PurpleRequestFields* fields, const char* ok_text,
							GCallback ok_cb, const char* cancel_text,
							GCallback cancel_cb, PurpleAccount* account,
							const char* who, PurpleConversation* conv,
							void* user_data);
	 static void*		ui_op_request_file(const char* title,
							const char* filename, gboolean savedialog,
							GCallback ok_cb, GCallback cancel_cb,
							PurpleAccount* account, const char* who,
							PurpleConversation* conv, void* user_data);
	 static void*		ui_op_request_folder(const char* title,
							const char* dirname, GCallback ok_cb,
							GCallback cancel_cb, PurpleAccount* account,
							const char* who, PurpleConversation* conv,
							void* user_data);
	 static void*		ui_op_request_action_with_icon(const char* title,
							const char* primary, const char* secondary,
							int default_action, PurpleAccount* account,
							const char* who, PurpleConversation* conv,
							gconstpointer icon_data, gsize icon_size,
							void* user_data, size_t action_count,
							va_list actions);

// Notify ui ops
	 static void*		ui_op_notify_message(PurpleNotifyMsgType type,
							const char* title, const char* primary,
							const char* secondary);

// Callbacks
	 static void		callback_set_public_alias_failure(PurpleAccount* account,
							const char* error);

// Util
			bool		is_own_user(PurpleAccount* account, const char* name);

			void		load_account_buddies(PurpleAccount* account);

			void		send_own_info(PurpleAccount* account);
			void		send_user_role(PurpleConversation* conv,
							const char* name, PurpleConvChatBuddyFlags flags);

PurpleStatusPrimitive	cardie_status_to_purple(UserStatus status);
		UserStatus		purple_status_to_cardie(PurpleStatus* status);

		const char*		purple_connection_error_name(
							const PurpleConnectionErrorInfo* error);

		const char*		purple_cache();
		const char*		account_cache(PurpleAccount* account);
		const char*		buddies_cache(PurpleAccount* account);
		const char*		buddy_cache(PurpleBuddy* buddy);

			void		purple_plugins_add_finddir(directory_which finddir);

			void		update_buddy(const char* path, BString user_id,
							BString user_name);

	static gboolean		_purple_glib_io_invoke(GIOChannel *source,
							GIOCondition condition, gpointer data);

		const char*		_tr(const char* string);

#endif // _PURPLE_APP_H
