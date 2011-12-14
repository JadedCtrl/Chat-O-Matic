/*
 * Copyright 2010-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_MESSAGES_H
#define _CAYA_MESSAGES_H

//! Show settings window
const uint32 CAYA_SHOW_SETTINGS = 'RPST';

//! Open chat window
const uint32 CAYA_OPEN_CHAT_WINDOW = 'CYow';

//! Close chat window
const uint32 CAYA_CLOSE_CHAT_WINDOW = 'CYcw';

//! Chat messages
const uint32 CAYA_CHAT = 'CYch';

//! Send replicant's messenger to Caya
const uint32 CAYA_REPLICANT_MESSENGER = 'RPme';

//! Status notification from the replicant
const uint32 CAYA_REPLICANT_STATUS_SET = 'RPMS';

//! Exit notification from replicant
const uint32 CAYA_REPLICANT_EXIT = 'RPEX';

//! Show main window replicant notification
const uint32 CAYA_REPLICANT_SHOW_WINDOW = 'CYSW';

#endif	// _CAYA_MESSAGES_H
