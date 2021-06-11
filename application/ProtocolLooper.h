/*
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_LOOPER_H
#define _PROTOCOL_LOOPER_H

#include <Looper.h>
#include <String.h>

#include <libsupport/KeyMap.h>

#include "CayaProtocol.h"

class Contact;
class Conversation;
class ConversationAccountItem;
class User;


typedef KeyMap<BString, Conversation*> ChatMap;
typedef KeyMap<BString, Contact*> RosterMap;
typedef KeyMap<BString, User*> UserMap;


class ProtocolLooper : public BLooper {
public:		
							ProtocolLooper(CayaProtocol* protocol, int64 instance);

			void			MessageReceived(BMessage* msg);

			CayaProtocol*	Protocol();

			ChatMap			Conversations() const;
			Conversation*	ConversationById(BString id);
			void			AddConversation(Conversation* chat);
			void			RemoveConversation(Conversation* chat);

			RosterMap		Contacts() const;
			Contact*		ContactById(BString id);
			void			AddContact(Contact* contact);

			UserMap			Users() const;
			User*			UserById(BString id);
			void			AddUser(User* user);

			BString			GetOwnId();
			void			SetOwnId(BString user_id);

			int64			GetInstance();

			ConversationAccountItem*
							GetListItem();

private:
			CayaProtocol*	fProtocol;
			int64			fInstance;

			BString			fMySelf;

			ChatMap			fChatMap;
			RosterMap		fRosterMap;
			UserMap			fUserMap;

			ConversationAccountItem*
							fListItem;
};

#endif	// _PROTOCOL_LOOPER_H
