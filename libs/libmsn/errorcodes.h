#ifndef __msn_errorcodes_h__
#define __msn_errorcodes_h__

/*
 * errorcodes.h
 * libmsn
 *
 * Created by Mark Rowe on Mon Mar 22 2004.
 * Refactored by Tiago Salem Herrmann on 08/2007.
 * Copyright (c) 2004 Mark Rowe. All rights reserved.
 * Copyright (c) 2007 Tiago Salem Herrmann. All rights reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <string>
#include <list>
#include <switchboardserver.h>

#include "libmsn_export.h"

/** \mainpage libmsn Reference
 *
 * <code>libmsn</code> is a C++ library for Microsoft's MSN Messenger service. It
 * provides a high-level interface that allows an application to access instant
 * messaging features with ease.  For more information, please visit the
 * <a href='http://libmsn.sourceforge.net'><code>libmsn</code></a> homepage.
 *
 */


/** Contains all of the functionality provided by <code>libmsn</code>.
 */
namespace MSN
{
    /** Error codes that the MSN servers may return in response to commands.
     */
    typedef enum 
    {
        ERR_SYNTAX_ERROR = 200,
        ERR_INVALID_PARAMETER,
        
        ERR_INVALID_USER = 205,
        ERR_FQDN_MISSING,
        ERR_ALREADY_LOGIN,
        ERR_INVALID_USERNAME,
        ERR_INVALID_FRIENDLY_NAME,
        ERR_LIST_FULL,
        
        ERR_ALREADY_THERE = 215,
        ERR_NOT_ON_LIST,
        
        ERR_ALREADY_IN_THE_MODE = 218,
        ERR_ALREADY_IN_OPPOSITE_LIST,
        
        ERR_SWITCHBOARD_FAILED = 280,
        ERR_NOTIFY_XFR_FAILED,
        
        ERR_REQUIRED_FIELDS_MISSING = 300,
        ERR_NOT_LOGGED_IN = 302,
        
        ERR_INTERNAL_SERVER = 500,
        ERR_DB_SERVER = 501,
        
        ERR_FILE_OPERATION = 510,
        ERR_MEMORY_ALLOC = 520,
        
        ERR_SERVER_BUSY = 600,
        ERR_SERVER_UNAVAILABLE,
        ERR_PEER_NS_DOWN,
        ERR_DB_CONNECT,
        ERR_SERVER_GOING_DOWN,
        
        ERR_CREATE_CONNECTION = 707,
        
        ERR_BLOCKING_WRITE = 711,
        ERR_SESSION_OVERLOAD,
        ERR_USER_TOO_ACTIVE,
        ERR_TOO_MANY_SESSIONS,
        ERR_NOT_EXPECTED,
        ERR_BAD_FRIEND_FILE = 717,
        
        ERR_AUTHENTICATION_FAILED = 911,
        ERR_NOT_ALLOWED_WHEN_OFFLINE = 913,
        ERR_NOT_ACCEPTING_NEW_USERS = 920
    } ErrorCodes;
}

#endif
