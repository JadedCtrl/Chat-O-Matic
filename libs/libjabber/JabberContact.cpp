/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */

#include <stdio.h>

#include "JabberContact.h"
#include "Logger.h"

JabberContact::JabberContact()
	: fPresence(new JabberPresence()),
	fId(""), fVCard(NULL)
{
}


JabberContact::~JabberContact() 
{
}


void
JabberContact::PrintToStream() 
{
	logmsg("JabberContact:");
	logmsg("     Name:  %s", fName.String());
	logmsg("    Group:  %s", fGroup.String());
	logmsg("      Jid:  %s", fJid.String());	
}


void
JabberContact::SetName(const BString& name) 
{
	fName = name;
}


void
JabberContact::SetGroup(const BString& group) 
{
	fGroup = group;
}


void
JabberContact::SetPresence(JabberPresence* presence) 
{
	fPresence = presence;
}


void
JabberContact::SetPresence()
{
	delete fPresence;
	fPresence = new JabberPresence();
}


void
JabberContact::SetJid(const BString& jid) 
{
	fJid = jid;
}


void
JabberContact::SetVCard(JabberVCard* vCard)
{
	fVCard = vCard;
}


void
JabberContact::SetSubscription(const BString& subscription) 
{
	fSubscription = subscription;
}


BString
JabberContact::GetSubscription() const
{
	return fSubscription;
}


BString
JabberContact::GetName() const
{
	return fName;
}


BString
JabberContact::GetGroup() const
{
	return fGroup;
}

JabberPresence*
JabberContact::GetPresence()
{
	return fPresence;
}


BString
JabberContact::GetJid() const
{
	return fJid;
}


JabberVCard*
JabberContact::GetVCard() const
{
	return fVCard;
}
			

BString
JabberContact::GetLastMessageID() const
{
	return fId;
}


void
JabberContact::SetLastMessageID(const BString& id)
{
	fId = id;
}
