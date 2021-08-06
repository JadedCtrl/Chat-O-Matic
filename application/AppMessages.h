/*
 * Copyright 2010-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _APP_MESSAGES_H
#define _APP_MESSAGES_H

//! Show settings window
const uint32 APP_SHOW_SETTINGS = 'RPST';

//! Open chat window
const uint32 APP_OPEN_CHAT_WINDOW = 'CYow';

//! Close chat window
const uint32 APP_CLOSE_CHAT_WINDOW = 'CYcw';

//! Chat messages
const uint32 APP_CHAT = 'CYch';

//! Create a new chat
const uint32 APP_NEW_CHAT = 'CYnc';

//! Create a new chat
const uint32 APP_NEW_ROOM = 'CYnr';

//! Join a chat
const uint32 APP_JOIN_ROOM = 'CYjr';

//! Invite user to current chat
const uint32 APP_SEND_INVITE = 'CYin';

//! Send replicant's messenger to the app
const uint32 APP_REPLICANT_MESSENGER = 'RPme';

//! Status notification from the replicant
const uint32 APP_REPLICANT_STATUS_SET = 'RPMS';

//! Exit notification from replicant
const uint32 APP_REPLICANT_EXIT = 'RPEX';

//! Show main window replicant notification
const uint32 APP_REPLICANT_SHOW_WINDOW = 'CYSW';

//! Select the upward conversation
const uint32 APP_MOVE_UP = 'CYmu';

//! Select the downward conversation
const uint32 APP_MOVE_DOWN = 'CYmd';

//! An account has been disabled
const uint32 APP_ACCOUNT_DISABLED = 'Axwo';

//! An account's initial connection failed
const uint32 APP_ACCOUNT_FAILED = 'Axwx';

//! Request a "help" message
const uint32 APP_REQUEST_HELP = 'CYhm';

//! Display a "user info" window
const uint32 APP_USER_INFO = 'CYuw';

//! Display a "room info" window
const uint32 APP_ROOM_INFO = 'CYrw';

//! Edit the contact roster
const uint32 APP_EDIT_ROSTER = 'CYer';

#endif	// _APP_MESSAGES_H
