/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_LOOPER_H
#define _PROTOCOL_LOOPER_H

#include <Looper.h>
#include <ObjectList.h>
#include <String.h>

#include <libsupport/KeyMap.h>

#include "CayaProtocol.h"
#include "ChatCommand.h"

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
							~ProtocolLooper();

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

			CommandMap		Commands() const;
			ChatCommand*	CommandById(BString id);
			void			AddCommand(ChatCommand* cmd);

			BObjectList<BMessage>
							UserPopUpItems() const;
			void			AddUserPopUpItem(BMessage* archived);

			BObjectList<BMessage>
							ChatPopUpItems() const;
			void			AddChatPopUpItem(BMessage* archived);

			BObjectList<BMessage>
							MenuBarItems() const;
			void			AddMenuBarItem(BMessage* archived);

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

			CommandMap		fCommands;
			BObjectList<BMessage> fUserItems;
			BObjectList<BMessage> fChatItems;
			BObjectList<BMessage> fMenuItems;

			ConversationAccountItem*
							fListItem;
};

#endif	// _PROTOCOL_LOOPER_H
