/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 */
#ifndef _JABBER_PROTOCOL_H
#define _JABBER_PROTOCOL_H

#include "JabberHandler.h"

class JabberProtocol : public JabberHandler {
public:
						JabberProtocol();
	virtual				~JabberProtocol();

	virtual const char*	Signature() const;
	virtual const char*	FriendlySignature() const;

	virtual BBitmap*	Icon() const;

	virtual BMessage	SettingsTemplate(const char* name);
	virtual	void		OverrideSettings();

	virtual	BString		ComposeJID() const;
};

#endif	// _JABBER_PROTOCOL_H
