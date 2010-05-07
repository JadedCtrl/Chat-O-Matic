/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */
#ifndef _JABBER_AGENT_H
#define _JABBER_AGENT_H

#include <String.h>

class JabberAgent {
public:
					JabberAgent();
					~JabberAgent();

	void			PrintToStream();

	bool 			HasGroupChat();
	bool 			Searchable();
	bool 			IsTransport();
	bool 			AllowsRegistration();

	BString 		GetService() const;
	BString 		GetName() const;
	BString 		GetJid() const;
	BString			GetInstructions() const;

	void 			SetGroupChat(bool groupChat);
	void 			SetSearchable(bool searchable);
	void 			SetTransport(bool transport);
	void 			SetRegistration(bool registration);
	void 			SetService(const BString& service);
	void 			SetName(const BString& name);
	void 			SetJid(const BString& jid);

private:
	bool 			fGroupChat;
	bool 			fSearchable;
	bool 			fTransport;
	bool 			fRegistration;
	BString 		fService;
	BString			fName;
	BString 		fJid;
};

#endif	// _JABBER_AGENT_H
