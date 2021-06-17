/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_SETTINGS_H
#define _PROTOCOL_SETTINGS_H

#include <ObjectList.h>
#include <String.h>

#include "ProtocolTemplate.h"

class BMessage;
class CayaProtocolAddOn;

class ProtocolSettings {
public:
						ProtocolSettings(CayaProtocolAddOn* addOn);

	status_t			InitCheck() const;

	CayaProtocolAddOn*	AddOn() const;
	BObjectList<BString> Accounts() const;

	status_t			Load(const char* account, BView* parent);
	status_t			Save(const char* account, BView* parent);

	status_t			Rename(const char* from, const char* to);
	status_t			Delete(const char* account);

private:
	status_t			_Load(const char* account, BMessage** settings);

	ProtocolTemplate	fTemplate;
	status_t			fStatus;
};

#endif	// _PROTOCOL_SETTINGS_H
