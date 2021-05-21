/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the GPL v2 License.
 */
#ifndef _FACEBOOK_PROTOCOL_H
#define _FACEBOOK_PROTOCOL_H

#include "JabberHandler.h"

class FacebookProtocol : public JabberHandler {
public:
						FacebookProtocol();
	virtual				~FacebookProtocol();

	virtual const char*	Signature() const;
	virtual const char*	FriendlySignature() const;

	virtual BBitmap*	Icon() const;

	virtual BMessage	SettingsTemplate();
	virtual	void		OverrideSettings();

	virtual	BString		ComposeJID() const;
};

#endif	// _FACEBOOK_PROTOCOL_H
