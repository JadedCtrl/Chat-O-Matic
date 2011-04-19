/*
 * This is a plugin ported from im_kit to Caya,
 * the code was updated to support libyahoo2.
 *
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
#include "YahooConnection.h"

#include <stdio.h>
#include <algorithm>

#include "CayaConstants.h"

#include <yahoo2.h>

#include "yahoo_util.h"

std::map<int, YahooConnection*> gYahooConnections;

YahooConnection::YahooConnection(YahooManager* mgr, const char* yahooID, const char* pass)
	:
	fManager(mgr),
	fID(-1),
	fYahooID(strdup(yahooID)),
	fPassword(strdup(pass)),
	fStatus(YAHOO_STATUS_OFFLINE),
	fAlive(true),
	fGotBuddyList(false)
{
	fThread = spawn_thread(
		yahoo_io_thread,
		"Yahoo IO",
		B_NORMAL_PRIORITY,
		this
	);

	resume_thread( fThread );
}


YahooConnection::~YahooConnection()
{
	LogOff();
	
	fAlive = false;
	int32 thread_res=0;
	
	wait_for_thread(fThread, &thread_res);
	
	free(fYahooID);
	free(fPassword);
}


void
YahooConnection::SetAway(enum yahoo_status state, const char *msg)
{
	if (fStatus == YAHOO_STATUS_OFFLINE) {
		fManager->Error("Calling SetAway() when offline", NULL);
		return;
	}
		yahoo_set_away(fID, state, msg, 1);
		//fManager->SetAway(state, msg);
}


void
YahooConnection::LogOff()
{
	if (fID < 0)
		return;

	yahoo_logoff(fID);

	fStatus = YAHOO_STATUS_OFFLINE;
	fID = -1;

	fManager->LoggedOut();

	// owner should delete us now to stop the thread and clean up
}


void
YahooConnection::Message(const char* who, const char* msg)
{
	if (fStatus == YAHOO_STATUS_OFFLINE) {
		fManager->Error( "Can't send message, not connected", who );
		return;
	}

	yahoo_send_im( 
		fID, 
		NULL /* default identity */,
		who,
		msg,
		1 /* it's utf-8 */,
		0
	);

	fManager->MessageSent( who, msg );
}


void
YahooConnection::AddBuddy(const char* who)
{
	if (!fGotBuddyList) {
		fBuddiesToAdd.push_back( who );
		return;
	}

	std::list<std::string>::iterator iter = find( fBuddies.begin(), fBuddies.end(), who );

	if (iter == fBuddies.end()) { // not in list, adding
		yahoo_add_buddy(fID, who, "Communicator", "");
		fBuddies.push_back(who);
		printf("Yahoo: Added buddy\n");
	}
}


void
YahooConnection::RemoveBuddy( const char* who )
{
	if (fStatus == YAHOO_STATUS_OFFLINE) {
		fManager->Error("Not connected when calling YahooConnection::RemoveBuddy()", NULL);
		return;
	}

	yahoo_remove_buddy( fID, who, "" );
}

void
YahooConnection::Typing(const char* who, int stat )
{
	yahoo_send_typing(fID,NULL,who,stat);
}

void
YahooConnection::GetBuddyIcon(const char*  who)
{
	yahoo_buddyicon_request(fID, who);
}


void
YahooConnection::cbStatusChanged(const char*  who, int stat, const char*  msg, int away, int idle)
{
	fManager->BuddyStatusChanged(who, stat, msg, away, idle);
}


void
YahooConnection::cbGotBuddies(YList*  buds)
{
	fGotBuddyList = true;

	std::list<struct yahoo_buddy> yabs;

	// copy from buds to buddies...
	for(; buds; buds = buds->next) {
		struct yahoo_buddy* bud = (struct yahoo_buddy* )buds->data;
		if (bud->real_name)
			yabs.push_back( *bud );
		fBuddies.push_back( bud->id );
		
		//yahoo_buddyicon_request(fID, bud->id);
	}
	printf("id %d\n", fID);
	// add waiting buddies
	for ( std::list<std::string>::iterator iter=fBuddiesToAdd.begin(); 
		iter != fBuddiesToAdd.end(); iter++) {
		AddBuddy( (*iter).c_str() );
	}
	fBuddiesToAdd.clear();

	// Tell the manager!
	fManager->GotBuddyList( fBuddies );
	fManager->GotContactsInfo( yabs );
}


void
YahooConnection::cbGotIM(const char* who, const char* msg, long /*tm*/, int /*stat*/, int /*utf8*/)
{
	//parse_html(msg);
	fManager->GotMessage( who, msg );
}


void
YahooConnection::cbTypeNotify(const char* who, int stat)
{
	fManager->TypeNotify( who, stat );
}


void
YahooConnection::cbGotBuddyIcon(const char* who, long size, const char* icon)
{
	fManager->GotBuddyIcon(who,size,icon);
}


void
YahooConnection::cbLoginResponse(int succ, const char* url)
{
	printf("YahooConnection::cbLoginResponse %s\n", url);
	if(succ == YAHOO_LOGIN_OK) {
		fStatus = yahoo_current_status(fID);
		fManager->LoggedIn();
		return;
	} else if(succ == YAHOO_LOGIN_UNAME) {
		fManager->Error("Could not log into Yahoo service - username not recognised.  Please verify that your username is correctly typed.", NULL);
	} else if(succ == YAHOO_LOGIN_PASSWD) {
		fManager->Error("Could not log into Yahoo service - password incorrect.  Please verify that your password is correctly typed.", NULL);
	} else if(succ == YAHOO_LOGIN_LOCK) {
		fManager->Error("Could not log into Yahoo service.  Your account has been locked.\nVisit [url] to reactivate it.\n", NULL);
	} else if(succ == YAHOO_LOGIN_DUPL) {
		fManager->Error("You have been logged out of the yahoo service, possibly due to a duplicate login.", NULL);
	} else if(succ == YAHOO_LOGIN_SOCK) {
		fManager->Error("The server closed the socket.", NULL);
	} else {
		fManager->Error("Could not log in, unknown reason.", NULL);
	}
	
	fStatus = YAHOO_STATUS_OFFLINE;

	LogOff();
}


void
YahooConnection::cbYahooError( const char* err, int fatal)
{
	fManager->Error( err, NULL );
	
	if (fatal) {
		LogOff();
	}
}


void
YahooConnection::AddConnection( conn_handler*  c )
{
	fConnections.AddItem(c);
}


void
YahooConnection::RemoveConnection( conn_handler* c )
{
	fConnections.RemoveItem(c);
}


conn_handler* 
YahooConnection::ConnectionAt( int i )
{
	return (conn_handler*)fConnections.ItemAt(i);
}


int
YahooConnection::CountConnections()
{
	return fConnections.CountItems();
}


bool
YahooConnection::IsAlive()
{
	return fAlive;
}
