/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 */
#ifndef _GOOGLE_TALK_PROTOCOL_H
#define _GOOGLE_TALK_PROTOCOL_H

#include "JabberHandler.h"

class GoogleTalkProtocol : public JabberHandler {
public:
						GoogleTalkProtocol();
	virtual				~GoogleTalkProtocol();

	virtual	void		OverrideSettings();
	virtual	BString		ComposeJID() const;
};

#endif	// _GOOGLE_TALK_PROTOCOL_H
