/*
 * This is a plugin ported from im_kit to Caya,
 * the code was updated to support libyahoo2.
 * Copyright (C) 2010-2011, Barrett
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
 *
 */
#ifndef YAHOO_CONNECTION_H
#define YAHOO_CONNECTION_H

#include <map>
#include <OS.h>
#include <yahoo2.h>
#include <yahoo2_callbacks.h>
#include <List.h>
#include <String.h>

#include <openssl/ssl.h>

#include "YahooManager.h"

struct _conn {
	int fd;
	SSL *ssl;
	int use_ssl;
	int remove;
};

struct conn_handler {
	struct _conn *con;
	int id;
	int tag;
	yahoo_input_condition cond;
	void *data;
	int remove;
};

class YahooConnection
{
	public:
		YahooConnection(YahooManager *, const char *, const char *);
		~YahooConnection();
		
		void SetAway(enum yahoo_status state, const char* msg);
		void LogOff();
		void Message(const char* who, const char* msg);
		// Call this to add new contacts, but only after having received the
		// buddy list so we don't double-add contacts
		void AddBuddy(const char* who);
		void RemoveBuddy( const char * who);
		void Typing(const char*, int);
		void GetBuddyIcon(const char* who);

		// Stuff below this is for use by the yahoo lib interface only.
		// I could make it protected and add all them functions as friends,
		// but that'd be boring. Will do it 'later'.
		void cbStatusChanged(const char* who, int stat,
								const char* msg, int away, int idle);
		void cbGotBuddies(YList* buds);
		void cbGotIM(const char* who, const char* msg, long tm, int stat, int utf8);
		void cbLoginResponse(int succ, const char* url);
		void cbYahooError(const char* err, int fatal);
		void cbTypeNotify(const char* who, int stat);
		void cbGotBuddyIcon(const char* who, long size, const char* icon);
		
		void AddConnection(conn_handler*);
		void RemoveConnection(conn_handler*);
		conn_handler* ConnectionAt(int i);
		int CountConnections();
		
	private:
		friend int32 yahoo_io_thread(void *);

		YahooManager * fManager;

		int 		fID;
		char* 		fYahooID;
		char* 		fPassword;

		int 		fStatus;

		thread_id	fThread;
		bool 		fAlive;
		bool		IsAlive();

		BList		fConnections;

		std::list<std::string>	fBuddies;
		std::list<std::string>	fBuddiesToAdd;
		bool fGotBuddyList;
};

extern std::map<int, YahooConnection*> gYahooConnections;
extern void cb_yahoo_url_handle(int id, int fd, int error, const char *filename, unsigned long size, void *data);
extern int32 yahoo_io_thread( void * _data );
extern const char *  kProtocolName;

#endif
