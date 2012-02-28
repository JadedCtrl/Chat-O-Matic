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
#include "ContactLinker.h"

class CayaProtocol;
class RosterItem;
class ProtocolLooper;

typedef KeyMap<BString, ContactLinker*> RosterMap;
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

			RosterMap		RosterItems() const;
			RosterItem*		RosterItemForId(BString id);

			// TODO: there should be a contact for each account.
			ContactLinker*	GetOwnContact();

private:
			ProtocolLooper*	_LooperFromMessage(BMessage* message);
			ContactLinker*	_EnsureContactLinker(BMessage* message);
			void			_ReplicantStatusNotify(CayaStatus status);

			RosterMap		fRosterMap;
			ProtocolLoopers	fLoopers;
			ContactLinker*	fMySelf;
};

#endif	// _SERVER_H
