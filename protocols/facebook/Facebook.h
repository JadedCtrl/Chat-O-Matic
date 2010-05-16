/*
 * Copyright 2004-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
 
#ifndef CAYA_Facebook_H
#define CAYA_Facebook_H

#include <list>

#include <List.h>
#include <Messenger.h>
#include <String.h>

#include <libjabber/JabberHandler.h>
#include <libjabber/JabberManager.h>

#include "CayaProtocol.h"
#include "CayaConstants.h"

//class FacebookConnection;
class JabberSocketPlug;

#define RosterList BObjectList<JabberContact>
#define AgentList BObjectList<JabberAgent>

class Facebook : public JabberManager, public JabberHandler, public CayaProtocol {
public:

				 Facebook();
		virtual ~Facebook();

		// IM::Protocol part begins here
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
		// IM::Protocol part ends here

		virtual CayaProtocolMessengerInterface* MessengerInterface() const { return fServerMsgr; }

		// JabberManager part begins here
		virtual void Error( const char * message, const char * who );

		virtual void GotMessage( const char * from, const char * msg );
		virtual void MessageSent( const char * to, const char * msg );

		virtual void LoggedIn();
		virtual void SetAway(bool);
		virtual void LoggedOut();

		//virtual void GotBuddyList( std::list<string> & );
		virtual void BuddyStatusChanged( const char * who, CayaStatus status );
		virtual void BuddyStatusChanged( JabberContact* who );
		virtual void BuddyStatusChanged( JabberPresence* who );
		// JabberManager part ends here

private:
		JabberSocketPlug*	fPlug;
		CayaProtocolMessengerInterface*	fServerMsgr;

		BString		fUsername;
		BString		fServer;
		BString		fPassword;

		typedef std::list<BString>	StrList; // new buddy added when off-line.
		StrList*			fLaterBuddyList;

		//special client
		//StrList				fSpecialUID;
		BMessage				fSpecialUID;

		bool 	fRostered;
		bool 	fAgent;
		float	fPerc;
		bool	fFullLogged;

		void Progress( const char * id, const char * message, float progress );

		JabberContact*	getContact(const char* id);
		void			SendContactInfo(const char* id);
		void			SendContactInfo(const JabberContact* jid);
		void			SendBuddyIcon(const char* id);
		void			AddStatusString(JabberPresence* who ,BMessage* to);	

		void			CheckLoginStatus();

// Callbacks from JabberHandler
protected:
	virtual	void				Authorized();
	virtual void 				Message(JabberMessage * message); 
	virtual void 				Presence(JabberPresence * presence);
	virtual void 				Roster(RosterList * roster);
	virtual void 				Agents(AgentList * agents);
	virtual void 				Disconnected(const BString & reason) ;
	virtual void 				SubscriptionRequest(JabberPresence * presence) ;
	virtual void 				Registration(JabberRegistration * registration) ;	
	virtual	void				Unsubscribe(JabberPresence * presence);
	virtual	void				OwnContactInfo(JabberContact* contact);
	virtual void				GotBuddyPhoto(const BString & jid, const BString & imagePath);
};

extern const char* kProtocolSignature;
extern const char* kProtocolName;

#endif	// CAYA_Facebook_H
