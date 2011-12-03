/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_SETTINGS_H
#define _PROTOCOL_SETTINGS_H

#include <libsupport/List.h>

class BMessage;
class CayaProtocolAddOn;

class ProtocolSettings {
public:
						ProtocolSettings(CayaProtocolAddOn* addOn);
						~ProtocolSettings();

	status_t			InitCheck() const;

	CayaProtocolAddOn*	AddOn() const;
	List<BString>		Accounts() const;

	status_t			LoadTemplate(BView* parent);
	status_t			Load(const char* account, BView* parent);
	status_t			Save(const char* account, BView* parent);

	status_t			Rename(const char* from, const char* to);
	status_t			Delete(const char* account);

private:
	status_t			fStatus;
	CayaProtocolAddOn*	fAddOn;
	BString				fAccount;
	BMessage*			fTemplate;

	void				_Init();
	status_t			_Load(const char* account, BMessage** settings);
	status_t			_Save(const char* account, BMessage* settings);
};

#endif	// _PROTOCOL_SETTINGS_H
