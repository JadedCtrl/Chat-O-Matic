/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */

#include <stdio.h>

#include "JabberAgent.h"
#include "Logger.h"


JabberAgent::JabberAgent() 
	: fGroupChat(false),
	fSearchable(false),
	fTransport(false),
	fRegistration(false),
	fService(""),
	fName(""),
	fJid("")
{
}


JabberAgent::~JabberAgent()
{
}


void
JabberAgent::PrintToStream() 
{
	logmsg("JabberAgent:");
	logmsg("     Name:  %s", fName.String());
	logmsg("  Service:  %s", fService.String());
	logmsg("      Jid:  %s", fJid.String());	
}


bool
JabberAgent::HasGroupChat()
{
	return fGroupChat;
}


bool
JabberAgent::IsTransport()
{
	return fTransport;
}


bool
JabberAgent::Searchable()
{
	return fSearchable;
}


bool
JabberAgent::AllowsRegistration()
{
	return fRegistration;
}


BString
JabberAgent::GetService() const
{
	return fService;
}


BString
JabberAgent::GetName() const
{
	return fName;
}


BString
JabberAgent::GetJid() const
{
	return fJid;
}


void
JabberAgent::SetService(const BString& service)
{
	fService = service;
}


void
JabberAgent::SetName(const BString& name)
{
	fName = name;
}


void
JabberAgent::SetJid(const BString& jid)
{
	fJid = jid;
}


void
JabberAgent::SetGroupChat(bool groupChat)
{
	fGroupChat = groupChat;
}


void
JabberAgent::SetSearchable(bool searchable)
{
	fSearchable = searchable;
}


void
JabberAgent::SetTransport(bool transport)
{
	fTransport = transport;
}


void
JabberAgent::SetRegistration(bool registration)
{
	fRegistration = registration;
}
