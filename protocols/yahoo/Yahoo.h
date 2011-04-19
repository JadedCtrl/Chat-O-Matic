#ifndef CAYA_YAHOO_H
#define CAYA_YAHOO_H

#include <String.h>
#include <Messenger.h>

#include <CayaProtocol.h>
#include <CayaConstants.h>
#include <CayaProtocolMessages.h>

#include "YahooManager.h"

class YahooConnection;

class Yahoo : public CayaProtocol, public YahooManager
{
	public:
		Yahoo();
		virtual ~Yahoo();

		// Caya Protocol part begins here
		// messenger to im_server
		virtual status_t Init( CayaProtocolMessengerInterface* );

		// called before unloading from memory
		virtual status_t Shutdown();

		// process message
		virtual status_t Process( BMessage * );

		// Get name of protocol
		virtual const char * Signature() const;
		virtual const char * FriendlySignature() const;

		// settings changed
		virtual status_t UpdateSettings( BMessage * );

		// preferred encoding of messages
		virtual uint32 GetEncoding();
		virtual CayaProtocolMessengerInterface* MessengerInterface() const 
			{ return fServerMsgr; }

		virtual	uint32	Version() const;
		// Caya Protocol part ends here
		void Progress( const char * id, const char * message, float progress );
		// YahooManager part begins here
		virtual void Error( const char * message, const char * who );

		virtual void GotMessage( const char * from, const char * msg );
		virtual void MessageSent( const char * to, const char * msg );

		virtual void LoggedIn();
		virtual void SetAway(bool);
		virtual void LoggedOut();

		virtual void TypeNotify(const char * who,int stat);

		virtual void GotBuddyList( std::list<std::string> & );
		virtual void GotContactsInfo( std::list<struct yahoo_buddy> & );
		virtual void GotBuddyIcon(const char *who, long size, const char* icon);
		virtual void BuddyStatusChanged(const char * who, int status, const char* msg, int away, int idle);
		// YahooManager part ends here

	private:
		CayaProtocolMessengerInterface*	fServerMsgr;
		BString		fYahooID;
		BString		fPassword;

		YahooConnection *	fYahoo;
};

#endif
