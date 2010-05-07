/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_CONSTANTS_H
#define _CAYA_CONSTANTS_H

#include <GraphicsDefs.h>

/*
 * Color constants.
 */
const rgb_color CAYA_ORANGE_COLOR = {255, 186, 0, 255};
const rgb_color CAYA_GREEN_COLOR = {43, 134, 43, 255};
const rgb_color CAYA_RED_COLOR = {175, 1, 1, 255};
const rgb_color CAYA_WHITE_COLOR = {255, 255, 255, 255};
const rgb_color CAYA_BLACK_COLOR = {0, 0, 0, 255};
const rgb_color CAYA_SELSTART_COLOR = {254, 150, 57};
const rgb_color CAYA_SELEND_COLOR = {230, 113, 9};

/**
 * What-codes for messages.
 */
enum message_what_codes {
	// Used for all error messages
	IM_ERROR						= 'IMer',

	// Returned after a request has succeded
	IM_ACTION_PERFORMED				= 'IMok',

	// All client <> protocol communication uses the MESSAGE what-code.
	IM_MESSAGE						= 'IMme', 

	IM_SERVER_BASED_CONTACT_LIST	= 'IMsl'
};

/**
 * Valid codes for im_what field.
 */
enum im_what_code {
	// Request a server-side contact list from protocol
	IM_GET_CONTACT_LIST				= 1,

	// Send a message to a contact
	IM_SEND_MESSAGE					= 2,

	// Message has been sent to contact
	IM_MESSAGE_SENT					= 3,

	// Message received from contact
	IM_MESSAGE_RECEIVED				= 4,

	// Contact's status has changes
	IM_STATUS_CHANGED				= 5,

	// Server-side contact list received
	IM_CONTACT_LIST					= 6,

	// Change own nickname
	IM_SET_NICKNAME					= 7,

	// Change own status
	IM_SET_STATUS					= 8,

	// Retreive information on contact
	IM_GET_CONTACT_INFO				= 9,

	// Received information on contact
	IM_CONTACT_INFO					= 10,
	
	// Start listening to changes in these contact's statuses
	IM_REGISTER_CONTACTS			= 11,

	// Contact started typing
	IM_CONTACT_STARTED_TYPING		= 12,

	// Contact stopped typing
	IM_CONTACT_STOPPED_TYPING		= 13,

	// User started typing
	IM_USER_STARTED_TYPING			= 14,

	// User stopped typing
	IM_USER_STOPPED_TYPING			= 15,

	// Own status was chagned
	IM_STATUS_SET					= 16,

	// Authorization request received
	IM_AUTH_REQUEST					= 17,

	// Send authorization
	IM_SEND_AUTH_ACK				= 18,

	// Contact has been authorized
	IM_CONTACT_AUTHORIZED			= 19,

	// Request authorization from contact
	IM_REQUEST_AUTH					= 20,

	// Stop listening to status changes from these contacts
	IM_UNREGISTER_CONTACTS			= 21,

	// Progress message received, could be login sequence, file transfer etc...
	IM_PROGRESS						= 22,

	// Away message
	GET_AWAY_MESSAGE				= 23,
	AWAY_MESSAGE					= 24,

	// Protocols send this when they get a new avatar icon
	IM_SET_AVATAR					= 25,

	// Client get this when an avatar icon is changed
	IM_AVATAR_CHANGED				= 26,

	// Adding and removing contact from the server side list
	IM_SERVER_LIST_ADD_CONTACT		= 27,
	IM_SERVER_LIST_REMOVED_CONTACT	= 28,

	// Get account contact information
	IM_OWN_CONTACT_INFO				= 29,

	// These are forwarded to or from protocols
	IM_SPECIAL_TO_PROTOCOL			= 1000,
	IM_SPECIAL_FROM_PROTOCOL		= 1001
};

enum CayaStatus {
	CAYA_ONLINE				= 1,
	CAYA_AWAY				= 2,
	CAYA_EXTENDED_AWAY		= 3,
	CAYA_DO_NOT_DISTURB		= 4,
	CAYA_OFFLINE			= 5,
	CAYA_STATUSES			= 6
};

#endif	// _CAYA_CONSTANTS_H
