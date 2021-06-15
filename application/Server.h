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
#include "ChatCommand.h"
#include "Contact.h"
#include "Conversation.h"
#include "ProtocolLooper.h"
#include "User.h"

class CayaProtocol;
class RosterItem;
class ProtocolLooper;


typedef KeyMap<bigtime_t, ProtocolLooper*> ProtocolLoopers;
typedef KeyMap<BString, bigtime_t> AccountInstances;


class Server: public BMessageFilter {
public:
							Server();
			void			Quit();
			void			LoginAll();

	virtual	filter_result	Filter(BMessage* message, BHandler** target);
			filter_result	ImMessage(BMessage* msg);

			void			AddProtocolLooper(bigtime_t instanceId,
								CayaProtocol* cayap);
			void			RemoveProtocolLooper(bigtime_t instanceId);
			ProtocolLooper*	GetProtocolLooper(bigtime_t instanceId);

			AccountInstances
							GetAccounts();

			void			SendProtocolMessage(BMessage* msg);
			void			SendAllProtocolMessage(BMessage* msg);

			RosterMap		Contacts() const;
			Contact*		ContactById(BString id, int64 instance);
			void			AddContact(Contact* contact, int64 instance);

			UserMap			Users() const;
			User*			UserById(BString id, int64 instance);
			void			AddUser(User* user, int64 instance);

			ChatMap			Conversations() const;
			Conversation*	ConversationById(BString id, int64 instance);
			void			AddConversation(Conversation* chat, int64 instance);

			CommandMap		Commands();
			ChatCommand*	CommandById(BString id);

private:
			ProtocolLooper*	_LooperFromMessage(BMessage* message);

			Contact*		_EnsureContact(BMessage* message);
			User*			_EnsureUser(BMessage* message);
			User*			_EnsureUser(BString id, ProtocolLooper* protoLooper);
			Conversation*	_EnsureConversation(BMessage* message);

			Role*			_GetRole(BMessage* msg);

			void			_InitDefaultCommands();
			void			_ReplicantStatusNotify(CayaStatus status);

			ProtocolLoopers	fLoopers;
			AccountInstances
							fAccounts;
			CommandMap		fCommands;
			BString			fMySelf;
};


#endif	// _SERVER_H

