/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _SERVER_H
#define _SERVER_H
 
#include <Message.h>
#include <MessageFilter.h>

#include <libsupport/KeyMap.h>

#include "CayaConstants.h"
#include "Contact.h"
#include "Conversation.h"

class CayaProtocol;
class RosterItem;
class ProtocolLooper;

typedef KeyMap<BString, Contact*> RosterMap;
typedef KeyMap<BString, Conversation*> ChatMap;
typedef KeyMap<bigtime_t, ProtocolLooper*> ProtocolLoopers;

class Server: public BMessageFilter {
public:
							Server();

	virtual	filter_result	Filter(BMessage* message, BHandler** target);
			filter_result	ImMessage(BMessage* msg);

			void			Quit();

			void			AddProtocolLooper(bigtime_t instanceId,
								CayaProtocol* cayap);
			void			RemoveProtocolLooper(bigtime_t instanceId);

			void			LoginAll();

			void			SendProtocolMessage(BMessage* msg);
			void			SendAllProtocolMessage(BMessage* msg);

			RosterMap		Contacts() const;
			Contact*		ContactById(BString id);
			void			AddContact(Contact* contact);

			ChatMap			Conversations() const;
			Conversation*	ConversationById(BString id);
			void			AddConversation(Conversation* chat);

			// TODO: there should be a contact for each account.
			Contact*	GetOwnContact();

private:
			ProtocolLooper*	_LooperFromMessage(BMessage* message);
			Contact*		_GetContact(BMessage* message);
			Contact*		_EnsureContact(BMessage* message);
			Conversation*	_EnsureConversation(BMessage* message);
			void			_ReplicantStatusNotify(CayaStatus status);

			RosterMap		fRosterMap;
			ChatMap			fChatMap;
			ProtocolLoopers	fLoopers;
			Contact*		fMySelf;
};

#endif	// _SERVER_H
