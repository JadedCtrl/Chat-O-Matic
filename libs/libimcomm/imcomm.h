/**  _
 ** (_)_ __  __ ___ _ __  _ __
 ** | | '  \/ _/ _ \ '  \| '  \
 ** |_|_|_|_\__\___/_|_|_|_|_|_|
 **
 ** Copyright (C) 2003-2005, Claudio Leite
 ** All rights reserved.
 **
 ** Please see the file 'COPYING' for licensing information.
 **/

#ifndef IMCOMM_H
#define IMCOMM_H

#ifdef PLAN9
#define _POSIX_SOURCE
#define _BSD_EXTENSION
#define _POSIX_EXTENSION
#include <errno.h>

/**
 ** On 386, this is correct. This may not be
 ** correct on other architectures.
 **/

#define uint32_t unsigned long
#define uint16_t unsigned short
#define uint8_t  unsigned char
#define int32_t long
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#define snprintf _snprintf
#else
#include <unistd.h>
#endif

#if defined(WATCOM_WIN32) || defined(_MSC_VER)
#define __MINGW32__		/* works for now */
#endif

#ifdef MACINTOSH_CLASSIC
#include <Types.h>
#include <MacTCP.h>
#include <AddressXlation.h>
#include <time.h>
#else
#ifndef __MINGW32__
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#ifndef __BEOS__
#include <arpa/inet.h>
#endif
#include <netdb.h>
#else
#include <winsock.h>

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int8  int8_t;
typedef unsigned __int8 uint8_t;
#else
#include <stdint.h>
#endif

#include <time.h>
#endif
#endif

#ifdef __BEOS__
#include <inttypes.h>
#endif

#define MD5_LOGIN

#ifdef MACINTOSH_CLASSIC
typedef void    fd_set;
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
char           *strdup(char *);
char           *strcasecmp(char *, char *);
void            mactcp_close(void *handle);
OSErr           mactcp_recv(void *handle, char *inbuf, size_t len);
#endif

#ifdef __AMIGA__
#define uint32_t u_int32_t
#define uint16_t u_int16_t
#define uint8_t u_int8_t
#endif

/*
 * Read the whole packet at once rather than one byte at a time.
 *
 * This is necessary for DOS, but all other platforms seem to be OK with reading
 * one byte at a time.
 *
 * UPDATED 0.80: I'm making this the default. I have yet to see an instance
 * where this is unacceptable, but yet I've seen plenty of occasions where
 * not having it slows old machines down considerably.
 *
 * Full packet may be frustrating on slow connections?
 */
#define FULL_PACKET_AT_ONCE

#ifndef __DJGPP__
#define IMCOMM_KEEPALIVE
#endif

#define NO_AUTO_IDLE

#ifdef linux
#include <time.h>
#endif

#if defined(__APPLE__) || defined(linux)
#include <stdint.h>
#endif

#include <string.h>
#include <ctype.h>

#define HOST_BIG_ENDIAN 0
#define HOST_LITTLE_ENDIAN 1

#ifdef __DJGPP__
#ifndef __dj_stdint__h_
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
#endif
#endif

typedef int     IMCOMM_RET;

#define IMCOMM_RET_ERROR -1
#define IMCOMM_RET_OK 1

#define NUM_CALLBACKS 11
enum {
	IMCOMM_IM_SIGNON,
	IMCOMM_IM_SIGNOFF,
	IMCOMM_IM_BUDDYAWAY,
	IMCOMM_IM_BUDDYUNAWAY,
	IMCOMM_IM_IDLEINFO,
	IMCOMM_IM_INCOMING,
	IMCOMM_IM_PROFILE,
	IMCOMM_IM_AWAYMSG,
	IMCOMM_ERROR,
	IMCOMM_FORMATTED_SN,
	IMCOMM_HANDLE_DELETED
};

enum {
	PROXY_TYPE_NONE,
	PROXY_TYPE_HTTPS,
	PROXY_TYPE_SOCKS5
};

enum {
	PROXY_ERROR_AUTH,
	PROXY_ERROR_CONNECT,
	PROXY_ERROR_PROXYCONNECT,
	PROXY_ERROR_UNKNOWN
};

enum {
	IMCOMM_ERROR_DISCONNECTED,
	IMCOMM_STATUS_CONNECTED,
	IMCOMM_ERROR_INVALID_LOGIN,
	IMCOMM_ERROR_OTHER_SIGNON,
	IMCOMM_STATUS_AUTHDONE,
	IMCOMM_RATE_LIMIT_WARN,
	IMCOMM_WARN_PAUSE,
	IMCOMM_WARN_UNPAUSE,
	IMCOMM_STATUS_MIGRATIONDONE,
	IMCOMM_ERROR_USER_OFFLINE,
	IMCOMM_ERROR_PROXY
};

#ifdef SEND_QUEUES
typedef struct IMCommSendQ {
	unsigned char  *data;
	size_t          len;
	int             updateidle;
	uint8_t         channel;
	struct IMCommSendQ *next;
}               send_q;
#endif

typedef struct IMComm {
#ifdef MACINTOSH_CLASSIC
	StreamPtr       s;
	int             readable;
#endif
#ifdef __MINGW32__
	WSADATA         wsadata;
	SOCKET          socket;
#else
	int             socket;
#endif
#ifdef MD5_LOGIN
	char           *sn;
	char           *pw;
#endif

	int             proxymode;
	char           *proxyserver;
	uint16_t        proxyport;

	uint16_t        oscarport;

	unsigned char   header[6];
	uint8_t         header_pos;
	unsigned char  *data;
	uint16_t        data_pos;
	uint16_t        data_len;
	int             connected;
	int             srv_pause;
	int             to_delete;
	uint16_t        seqnum;
	uint32_t        snacreq;
	uint16_t        max_profile_len;
	uint16_t        max_capabilities;
	uint16_t        max_buddylist_size;
	uint16_t        max_num_watchers;
	uint16_t        max_online_notifications;
	uint16_t        max_message_size;
	uint16_t        max_sender_warning;
	uint16_t        max_receiver_warning;
	uint16_t        max_message_interval;
	uint16_t        max_visible_list_size;
	uint16_t        max_invisible_list_size;
	/* void (*callbacks[NUM_CALLBACKS]) (void *,...); */
	void            (*callbacks[NUM_CALLBACKS]) ();
	struct IMComm_Families *families;
	int             num_families;
	struct IMComm_BuddyList *buddylist;
	struct IMComm_BuddyList *buddies_online;
	int             isidle;
	int             isaway;
	int             isinvisible;
	long            last_operation_time;
	unsigned char  *profile_str;
	unsigned char  *away_msg;
#ifdef IMCOMM_KEEPALIVE
	long            last_keepalive_time;
#endif
#ifdef SEND_QUEUES
	send_q         *s_queue;
#endif
	int             ischild;
	void           *parent;
	uint8_t        *icondata;
	uint16_t        iconlen;
}               IMCOMM;

typedef struct IMComm_Families {
	uint16_t        family;
	struct IMComm_Families *next;
}               IMCOMM_FAMILIES;

typedef struct IMComm_BuddyList {
	char           *sn;
	char           *formattedsn;
	unsigned long   idletime;
	unsigned long   onlinetime;
	uint16_t        ssi_id;
	uint16_t        group_id;
	int             isaway;
	struct IMComm_BuddyList *next;
}               IMCOMM_BUDDYLIST;

typedef struct IMComm_Handles {
	IMCOMM         *handle;
	struct IMComm_Handles *next;
}               IMCOMM_HANDLES;

typedef struct TLVList {
	uint16_t        type;
	uint16_t        len;
	uint8_t        *value;
	struct TLVList *next;
}               TLVLIST;

typedef struct IMComm_Packet {
	unsigned char  *data;
	size_t          len;
	size_t          offset;
}               pkt_t;

struct MultiPacket {
	int             init;
	uint8_t         channel;
	unsigned char  *packet;
	size_t          len;
	int             updateidle;
	struct MultiPacket *next;
};

#include "protos.h"
#include "byteswap.h"
#endif
