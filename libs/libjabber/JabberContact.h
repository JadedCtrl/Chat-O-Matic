/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */
#ifndef _JABBER_CONTACT_H
#define _JABBER_CONTACT_H

#include <String.h>

#include "JabberPresence.h"
#include "JabberVCard.h"

class JabberContact {
public:
								JabberContact();
	virtual						~JabberContact();

			void 				SetName(const BString& name);
			void 				SetGroup(const BString& group);
			void 				SetSubscription(const BString& group);
	virtual	void				SetPresence();
	virtual void 				SetPresence(JabberPresence* presence);
			void 				SetJid(const BString& jid);
			void				SetVCard(JabberVCard* vCard);
			void				PrintToStream();
			BString 			GetName() const;
			BString 			GetGroup() const;
			BString 			GetSubscription() const;
			JabberPresence*		GetPresence();
			BString 			GetJid() const;
			JabberVCard*		GetVCard() const;

			BString				GetLastMessageID() const;
			void				SetLastMessageID(const BString& id);

private:
			BString				fJid;
			BString				fGroup;
			JabberPresence*		fPresence;
			BString				fName;
			BString				fId;
			BString				fSubscription;
			JabberVCard*		fVCard;
};

#endif	// JABBER_CONTACT_H
