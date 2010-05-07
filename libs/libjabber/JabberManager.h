/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef Jabber_MANAGER_H
#define Jabber_MANAGER_H

#include <list>
#include <string>

#include "CayaConstants.h"

/**
	
*/

class JabberManager
{
	public:
		// who can be NULL if it's a general message
		virtual void Error( const char * message, const char * who )=0;
		
		virtual void GotMessage( const char * from, const char * msg )=0;
		virtual void MessageSent( const char * to, const char * msg )=0;
		
		virtual void LoggedIn()=0;
		virtual void SetAway(bool)=0;
		virtual void LoggedOut()=0;
		
		//virtual void GotBuddyList( list<string> & )=0;
		virtual void BuddyStatusChanged( const char * who, CayaStatus status )=0;
};

#endif
