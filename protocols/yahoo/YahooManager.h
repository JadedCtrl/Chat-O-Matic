#ifndef YAHOO_MANAGER_H
#define YAHOO_MANAGER_H

#include <list>
#include <string>
#include <yahoo2_types.h>

/**
	
*/

class YahooManager
{
	public:
		// who can be NULL if it's a general message
		virtual void Error(const char * message, const char * who)=0;
		
		virtual void GotMessage(const char * from, const char * msg)=0;
		virtual void MessageSent(const char * to, const char * msg)=0;
		
		virtual void LoggedIn()=0;
		virtual void SetAway(bool)=0;
		virtual void LoggedOut()=0;
		
		virtual void TypeNotify(const char * who,int stat)=0;
		
		virtual void GotBuddyList(std::list<std::string> &)=0;
		virtual void GotContactsInfo(std::list<struct yahoo_buddy> &)=0;
		virtual void GotBuddyIcon(const char *who, long size, const char* icon)=0;
		virtual void BuddyStatusChanged(const char * who, int status, const char* msg, int away, int idle)=0;
};

#endif
