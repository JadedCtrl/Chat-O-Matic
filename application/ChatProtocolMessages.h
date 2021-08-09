/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHAT_PROTOCOL_MESSAGES_H
#define _CHAT_PROTOCOL_MESSAGES_H

/**
 * What-codes for messages.
 */
enum message_what_codes {
	//!	All client <> protocol communication uses this what-code
	IM_MESSAGE						= 'IMme',

	//!	Used for very important (blocking) error messages
	IM_ERROR						= 'IMer',

	//!	Returned after a request has succeded
	IM_ACTION_PERFORMED				= 'IMap'
};

/**
 * Valid codes for im_what field.
 */
enum im_what_code {
	/*
	 * Messages that involves server-side contact list.
	 */

	//!	Request a server-side contact list from protocol →Protocol
	IM_GET_ROSTER						= 1,

	/*!	Server-side contact list received →App
		Requires:	Stringlist "user_id" */
	IM_ROSTER							= 2,

	/*!	Add a contact to the roster		→Protocol
		The slots for this message are determined by the protocol's
		"roster" template (ChatProtocol::SettingsTemplate("roster")) */
	IM_ROSTER_ADD_CONTACT				= 3,

	/*!	Remove a contact				→Protocol
		Requires:	String "user_id" */
	IM_ROSTER_REMOVE_CONTACT			= 4,

	/*!	Contact(s) removed from the server-side list →App
		Requires:	String "user_id" */
	IM_ROSTER_CONTACT_REMOVED			= 5,

	/*! Edit some data on contact		→Protocol
		The slots for this message are determined by the protocol's
		"roster" template (ChatProtocol::SettingsTemplate("roster")) */
	IM_ROSTER_EDIT_CONTACT				= 6,


	/*
	 * Messages related to text chat.
	 */

	/*!	Send a chat message				→Protocol
		Requires:	String "user_id", String "body" */
	IM_SEND_MESSAGE						= 20,

	/*!	Chat message has been sent		→App
		If no user_id is specified, it's treated as a system message
		Requires:	String "chat_id", String "body"
		Allows:		String "user_id" */
	IM_MESSAGE_SENT						= 21,

	/*!	Chat message received			→App
		Requires:	String "chat_id", String "user_id", String "body" */
	IM_MESSAGE_RECEIVED					= 22,

	/*!	Logs received					→App
		Without "when" (a time_t), the logged message will lack a timestamp
		Requires:	Strings "chat_id", Strings "user_id", Strings "body"
		Accepts:	in64s "when" */
	IM_LOGS_RECEIVED					= 23,


	/*
	 * Messages related changes in general users.
	 */

	/*!	User's nick has changed			→App */
	IM_USER_NICKNAME_SET				= 40,

	/*!	Received new status for user	→App
		Requires:	String "user_id", int32/UserStatus "status" */
	IM_USER_STATUS_SET					= 41,

	/*!	User's avatar icon was changed	→App
		Requires:	String "user_id", Ref "ref" */
	IM_USER_AVATAR_SET					= 42,


	/*
	 * Messages related to contact's information received from protocols.
	 */

	/*!	Get contact information			→Protocol
		Requires:	String "user_id" */
	IM_GET_CONTACT_INFO					= 62,

	/*!	Received contact information	→App
		Requires:	String "user_id"
		Accepts:	String "user_name", String "message",
					int32/UserStatus "status" */
	IM_CONTACT_INFO						= 63,

	/*!	Request contact information		→Protocol
		Requires:	String "user_id" */
	IM_GET_EXTENDED_CONTACT_INFO		= 64,

	/*!	Received contact information	→App
		Requires:	String "user_id",
					non-standard slots used by "roster" template
		Accepts:	String "user_name" */
	IM_EXTENDED_CONTACT_INFO			= 65,


	/*
	 * Messages that involve changing own information.
	 */

	/*!	Change own nickname				→Protocol
		Requires:	String "user_name" */
	IM_SET_OWN_NICKNAME					= 80,

	/*!	Own nickname was changed		→App
		Requires:	String "user_name" */
	IM_OWN_NICKNAME_SET					= 81,

	/*!	Change own status				→Protocol
		Requires:	int32/UserStatus "status" */
	IM_SET_OWN_STATUS					= 82,

	/*!	Own status was changed			→App
		Requires:	int32/UserStatus "status" */
	IM_OWN_STATUS_SET					= 83,

	/*!	Get own contact information		→App
		Must be send right after connection is established,
		before any room events, etc.
		Requires:	String "user_id"
		Allows:		String "user_name" */
	IM_OWN_CONTACT_INFO					= 84,

	//!	Change own avatar icon
	IM_SET_OWN_AVATAR					= 85,

	/*!	Own avatar icon was changed
		 Requires:	Ref "ref" */
	IM_OWN_AVATAR_SET					= 86,


	/*
	 * Contacts registration.
	 */

	//!	Start listening to changes in these contact's statuses [unused]
	IM_REGISTER_CONTACTS				= 100,

	//!	Stop listening to status changes from these contacts [unused]
	IM_UNREGISTER_CONTACTS				= 101,


	/*
	 * Authorization.
	 */

	//!	Ask authorization to contact [unused]
	IM_ASK_AUTHORIZATION				= 120,

	//!	Authorization response received from contact [unused]
	IM_AUTHORIZATION_RECEIVED			= 121,

	//!	Authorization request received from contact [unused]
	IM_AUTHORIZATION_REQUEST			= 122,

	//!	Authorization response given to contact [unused]
	IM_AUTHORIZATION_RESPONSE			= 123,

	//!	Contact has been authorized [unused]
	IM_CONTACT_AUTHORIZED				= 124,


	/*
	 * Miscellaneous.
	 */

	//!	Progress message received, could be login sequence, file transfer etc
	IM_PROGRESS							= 140,

	//!	Notifications
	IM_NOTIFICATION						= 141,


	/*
	 * Room membership
	 */

	/*!	Create an individual chat		→Protocol
		Individual chats and rooms are really the same thing (at least according
		to App)― the only difference is in how they're created and joined.
		A "chat" should be uniquely tied to a single user, and its chat_id
		should be derivable from the user's ID (when sent back from
		CHAT_CREATED). It doesn't matter how you get this done, really.
		Requires:	String "user_id" */
	IM_CREATE_CHAT						= 150,

	/*!	Chat has been created			→App
		Requires:	String "chat_id", String "user_id" */
	IM_CHAT_CREATED						= 151,

	/*!	Create a room					→Protocol
		The required slots for this message are completely determined by the
		protocol itself― the protocol will just receive data from the
		"create_room" template (which is fetched via
		ChatProtocol::SettingsTemplate("create_room") */
	IM_CREATE_ROOM						= 152,

	/*!	Inform App room was created	→App
		Just a semantically-dressed IM_ROOM_JOINED
		Requires:	String "chat_id" */
	IM_ROOM_CREATED						= 153,

	/*!	Join a room						→Protocol
		The required slots for this message are completely determined by the
		protocol itself― like IM_CREATE_ROOM― with the "join_room" template. */
	IM_JOIN_ROOM						= 154,

	/*!	Confirm the room's been joined	→App
		Requires:	String "chat_id" */
	IM_ROOM_JOINED						= 155,

	/*!	User wants to leave the room	→Protocol
		Requires:	String "chat_id" */
	IM_LEAVE_ROOM						= 156,

	/*!	User left the room				→App
		Requires:	String "chat_id" */
	IM_ROOM_LEFT						= 157,

	/*! Request a room's userlist		→Protocol
		Requires:	String "chat_id" */
	IM_GET_ROOM_PARTICIPANTS			= 158,

	/*!	Quietly add user(s) to the chat	→App
		Shouldn't be sent automatically on joining a room.
		Requires:	String "chat_id", StringList "user_id"
		Accepts:	StringList "user_name" */
	IM_ROOM_PARTICIPANTS				= 159,

	/*!	User has explicitly joined		→App
		 Requires:	String "chat_id", String "user_id"
		 Accepts:	String "body" */
	IM_ROOM_PARTICIPANT_JOINED			= 160,

	/*!	A user left the room			→App
		Requires:	String "chat_id", String "user_id"
		Accepts:	String "user_name", String "body" */
	IM_ROOM_PARTICIPANT_LEFT			= 161,

	/*!	Invite a user to a room			→Protocol
		You can tell it succeded with IM_ROOM_PARTICIPANT_JOINED.
		Requires:	String "chat_id", String "user_id"
		Accepts:	String "body" */
	IM_ROOM_SEND_INVITE					= 162,

	/*!	Invitee explicitly refused		→App
		Requires:	String "chat_id", String "user_id"
		Accepts:	String "user_name", String "body" */
	IM_ROOM_INVITE_REFUSED				= 163,

	/*!	User was invited to a room		→App
		Requires:	String "chat_id"
		Accepts:	String "user_id", String "chat_name", String "body" */
	IM_ROOM_INVITE_RECEIVED				= 164,

	/*!	User accepted an invite			→Protocol
		Requires:	String "chat_id" */
	IM_ROOM_INVITE_ACCEPT				= 165,

	/*!	User denies an invite			→Protocol
		Requires:	String "chat_id" */
	IM_ROOM_INVITE_REFUSE				= 166,


	/*
	 * Room metadata
	 */

	/*!	Request a room's metadata		→Protocol
		Requires:	String "chat_id" */
	IM_GET_ROOM_METADATA				= 170,

	/*!	Receive room metadata			→App
		The idea is that all other metadata-related messages should only be
		called either from a request, or from a change.
		This shouldn't be sent automatically upon joining a room.

		Recommendations on default room flags: Unless your protocol has remote
		logs, ROOM_LOG_LOCALLY and ROOM_POPULATE_LOGS should be enabled; and for
		multi-user rooms, ROOM_AUTOJOIN should be enabled by default (again,
		unless the protocol manages auto-joins).
		Requires:	String "chat_id"
		Allows:		String "chat_name", String "subject",
					int32 "room_default_flags", int32 "room_disallowed_flags" */
	IM_ROOM_METADATA					= 171,

	/*!	Set the room name				→Protocol
		Requires:	String "chat_id", String "chat_name" */
	IM_SET_ROOM_NAME					= 172,

	/*!	Room name has changed			→Protocol
		Requires:	String "chat_id", String "chat_name" */
	IM_ROOM_NAME_SET					= 173,

	/*!	Set the room subject			→App
		Requires:	String "chat_id", String "subject" */
	IM_SET_ROOM_SUBJECT					= 174,

	/*!	Subject has been changed		→App
		Requires:	String "chat_id", String "subject" */
	IM_ROOM_SUBJECT_SET					= 175,


	/*
	 * Room moderation
	 */

	/*!	A user's role has been changed	→App
		Requires:	String "role_title", int32 "role_perms",
					int32 "role_priority" */
	IM_ROOM_ROLECHANGED					= 190,

	/*!	Kick user						→Protocol
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_KICK_PARTICIPANT			= 191,

	/*!	A user was kicked				→App
		Requires:	String "chat_id", String "user_id"
		Accepts:	String "user_name", String "body" */
	IM_ROOM_PARTICIPANT_KICKED			= 192,

	/*!	Ban user						→Protocol
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_BAN_PARTICIPANT				= 193,

	/*!	A user was banned				→App
		Requires:	String "chat_id", String "user_id"
		Accepts:	String "user_name", String "body" */
	IM_ROOM_PARTICIPANT_BANNED			= 194,

	/*!	Unban user →Protocol */
	IM_ROOM_UNBAN_PARTICIPANT			= 195,

	/*!	Mute user						→Protocol
		The result of this can be seen with IM_ROOM_ROLECHANGED.
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_MUTE_PARTICIPANT			= 196,

	/*!	Unmute user						→Protocol
		The result of this can be seen with IM_ROOM_ROLECHANGED.
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_UNMUTE_PARTICIPANT			= 197,

	/*!	Deafen							→Protocol
		The result of this can be seen with IM_ROOM_ROLECHANGED.
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_DEAFEN_PARTICIPANT			= 198,

	/*!	Allow to read messages			→Protocol
		The result of this can be seen with IM_ROOM_ROLECHANGED.
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_UNDEAFEN_PARTICIPANT		= 199,


	/*
	 * Misc. room-related messages
	 */

	/*!	User started typing				→App [unused]
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_PARTICIPANT_STARTED_TYPING		= 210,

	/*!	User stopped typing				→App [unused]
		Requires:	String "chat_id", String "user_id" */
	IM_ROOM_PARTICIPANT_STOPPED_TYPING		= 211,


	/*
	 * Misc. UI messages
	 */

	/*! Reload commands from proto		→App
		If a protocol's command-list is dynamic (i.e., determined by its own
		add-ons and preferences), it makes sense to relaod commands from time
		to time. This forces that. */
	IM_PROTOCOL_RELOAD_COMMANDS			= 900,


	/*
	 * Special messages
	 */

	//!	Special message forwarded to protocol [unused]
	IM_SPECIAL_TO_PROTOCOL				= 1000,

	//!	Special message forwarded from protocol [unused]
	IM_SPECIAL_FROM_PROTOCOL			= 1001,

	/*!	Protocol is ready				→App
		Should be sent after connection is established, initialization done */
	IM_PROTOCOL_READY					= 1002,

	/*! Deletion of protocol requested	→App
		This requests that the app delete the ChatProtocol and its
		ProtocolLooper― so invoking ChatProtocol::Shutdown().
		This should be sent by the protocol after connection errors or a
		disconnect, when the addon doesn't have anything left to do other
		than yearn for the sweet hand of death. */
	IM_PROTOCOL_DISABLE					= 1003
};

#endif	// _CHAT_PROTOCOL_MESSAGES_H
