/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef _PURPLE_MESSAGES_H
#define _PURPLE_MESSAGES_H

#include <SupportDefs.h>

enum purple_message {
	/*
	 * Messages between the Purple add-on and server
	 */

	/*! Request a count of protocols.		→Server
	 *  Response is sent directly to the requesting thread
	 *  as a message's code, use receive_data() to catch it.
	 *  Requires:	int64 thread_id */
	PURPLE_REQUEST_PROTOCOL_COUNT			= 'PApc',

	/*! Request protocol metadata.			→Server
	 *  Response is sent directly to the requesting thread
	 *  with two subsquent messages (using receive_data())―
	 *  the first sending the size of the subsequently sent
	 *  flattened BMessage.
	 *  Requires:	int32 protocol_index, int64 thread_id */
	PURPLE_REQUEST_PROTOCOL_INFO			= 'PApi',

	/*! Load/start connecting the account	→Server
	 *	Just the account's settings message from Cardie's end.
	 *	It's the server's job to tie the Cardie account name
	 *	to the PurpleAccount. */
	PURPLE_LOAD_ACCOUNT						= 'PAla',

	/*! Associate account with thread		→Server
	 *	Makes the server associate the given account with
	 *	the given thread. All subsequent Server→Add-On
	 *	messages related to the account will be sent to this
	 *	thread.
	 *	Requires:	String account_name, int64 thread_id */
	PURPLE_REGISTER_THREAD					= 'PArl'
};

#endif // _PURPLE_MESSAGES_H
