/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _MATRIX_MESSAGES_H
#define _MATRIX_MESSAGES_H

enum matrix_message {
	/*
	 * Messages between the Matrix add-on and server
	 */

	/*! Register account & thread with app	→Server
		Requires:	Account template, int64 thread_id */
	MATRIX_REGISTER_ACCOUNT					= 'MXra',


	/*! Inform the protocol of app team id	→Protocol
		Requires:	int64 team_id */
	MATRIX_ACCOUNT_REGISTERED				= 'MXar'
};

#endif // _MATRIX_MESSAGES_H
