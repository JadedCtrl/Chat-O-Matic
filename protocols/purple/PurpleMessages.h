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
	 *  Response is sent directly to the requesting thread,
	 *  use receive_data() to catch it.
	 *  Requires:	int64 thread_id */
	PURPLE_REQUEST_PROTOCOL_COUNT			= 1,

	/*! Request protocol metadata.			→Server
	 *  Response is sent directly to the requesting thread,
	 *  use receive_data() to catch it.
	 *  Requires:	int32 protocol_index, int64 thread_id */
	PURPLE_REQUEST_PROTOCOL_INFO			= 2
};


#endif // _PURPLE_MESSAGESH
