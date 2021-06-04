/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PROTOCOL_MESSAGES_H
#define _CAYA_PROTOCOL_MESSAGES_H

/**
 * What-codes for messages.
 */
enum message_what_codes {
	//! All client <> protocol communication uses this what-code
	IM_MESSAGE						= 'IMme',

	//! Used for very important (blocking) error messages
	IM_ERROR						= 'IMer',

	//! Returned after a request has succeded
	IM_ACTION_PERFORMED				= 'IMap'
};

/**
 * Valid codes for im_what field.
 */
enum im_what_code {
	/*
	 * Messages that involves server-side contact list.
	 */

	//! Request a server-side contact list from protocol
	IM_GET_CONTACT_LIST					= 1,

	//! Server-side contact list received
	IM_CONTACT_LIST						= 2,

	//! Contact(s) was added to the server-side list
	IM_CONTACT_LIST_ADD_CONTACT			= 3,

	//! Contact(s) removed from the server-side list
	IM_CONTACT_LIST_REMOVED_CONTACT		= 4,


	/*
	 * Messages related to text chat.
	 */

	//! Send a chat message
	IM_SEND_MESSAGE						= 20,

	//! Chat message has been sent
	IM_MESSAGE_SENT						= 21,

	//! Chat message received
	IM_MESSAGE_RECEIVED					= 22,

	//! Contact started typing
	IM_CONTACT_STARTED_TYPING			= 23,

	//! Contact stopped typing
	IM_CONTACT_STOPPED_TYPING			= 24,

	//! Contact gone
	IM_CONTACT_GONE						= 25,

	//! User started typing
	IM_USER_STARTED_TYPING				= 26,

	//! User stopped typing
	IM_USER_STOPPED_TYPING				= 27,

	//! Logs received
	IM_LOGS_RECEIVED					= 28,


	/*
	 * Messages related to contact changes.
	 */

	//! Change contact's status
	IM_SET_NICKNAME						= 40,

	//! Contact's status has changed
	IM_NICKNAME_SET						= 41,


	/*
	 * Messages related to contact's information received from protocols.
	 */

	//! Received contact new status
	IM_STATUS_SET						= 60,

	//! Contact's avatar icon was changed
	IM_AVATAR_SET						= 61,

	//! Get contact information
	IM_GET_CONTACT_INFO					= 62,

	//! Received contact information
	IM_CONTACT_INFO						= 63,

	//! Get extended contact information
	IM_GET_EXTENDED_CONTACT_INFO		= 64,

	//! Received extended contact information
	IM_EXTENDED_CONTACT_INFO			= 65,


	/*
	 * Messages that involve changing own information.
	 */

	//! Change own nickname
	IM_SET_OWN_NICKNAME					= 80,

	//! Own nickname was changed
	IM_OWN_NICKNAME_SET					= 81,

	//! Change own status
	IM_SET_OWN_STATUS					= 82,

	// Own status was chagned
	IM_OWN_STATUS_SET					= 83,

	//! Get own contact information
	IM_OWN_CONTACT_INFO					= 84,

	//! Change own avatar icon
	IM_SET_OWN_AVATAR					= 85,

	//! Own avatar icon was changed
	IM_OWN_AVATAR_SET					= 86,


	/*
	 * Contacts registration.
	 */

	//! Start listening to changes in these contact's statuses
	IM_REGISTER_CONTACTS				= 100,

	//! Stop listening to status changes from these contacts
	IM_UNREGISTER_CONTACTS				= 101,


	/*
	 * Authorization.
	 */

	//! Ask authorization to contact
	IM_ASK_AUTHORIZATION				= 120,

	//! Authorization response received from contact
	IM_AUTHORIZATION_RECEIVED			= 121,

	//! Authorization request received from contact
	IM_AUTHORIZATION_REQUEST			= 122,

	//! Authorization response given to contact
	IM_AUTHORIZATION_RESPONSE			= 123,

	//! Contact has been authorized
	IM_CONTACT_AUTHORIZED				= 124,


	/*
	 * Miscellaneous.
	 */

	//! Progress message received, could be login sequence, file transfer etc...
	IM_PROGRESS							= 140,

	//! Notifications
	IM_NOTIFICATION						= 141,


	/*
	 * Room membership
	 */

	//! Create an individual chat
	IM_CREATE_CHAT						= 150,

	//! Chat has been created
	IM_CHAT_CREATED						= 151,

	//! Join a room
	IM_JOIN_ROOM						= 152,

	//! Confirm the room's been joined
	IM_ROOM_JOINED						= 153,

	//! User left the room
	IM_LEAVE_ROOM						= 154,

	//! User left the room
	IM_ROOM_LEFT						= 155,

	//! Quietly add a user(s) to the chat
	IM_ROOM_PARTICIPANTS				= 156,

	//! User has newly and explicitly joined
	IM_ROOM_PARTICIPANT_JOINED			= 157,

	//! A user left the room
	IM_ROOM_PARTICIPANT_LEFT			= 158,


	/*
	 * Room metadata
	 */

	//! Room name
	IM_ROOM_NAME						= 160,

	//! Room subject
	IM_ROOM_SUBJECT						= 161,


	/*
	 * Room moderation
	 */

	//! Ban user
	IM_ROOM_BAN_PARTICIPANT				= 170,

	//! Kick user
	IM_ROOM_KICK_PARTICIPANT			= 171,


	/*
	 * Special messages
	 */

	//! Special message forwarded to protocol
	IM_SPECIAL_TO_PROTOCOL				= 1000,

	//! Special message forwarded from protocol
	IM_SPECIAL_FROM_PROTOCOL			= 1001
};

#endif	// _CAYA_PROTOCOL_MESSAGES_H
