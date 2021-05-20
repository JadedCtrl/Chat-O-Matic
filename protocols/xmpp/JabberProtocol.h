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

	virtual	void		OverrideSettings();
	virtual BMessage	SettingsTemplate();
	virtual	BString		ComposeJID() const;
};

#endif	// _JABBER_PROTOCOL_H
