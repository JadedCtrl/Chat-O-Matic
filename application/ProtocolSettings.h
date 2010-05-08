/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_SETTINGS_H
#define _PROTOCOL_SETTINGS_H

#include <libsupport/List.h>

class BMessage;
class CayaProtocol;

class ProtocolSettings {
public:
						ProtocolSettings(CayaProtocol* cayap);
						~ProtocolSettings();

	status_t			InitCheck() const;

	CayaProtocol*		Protocol() const;
	List<BString>		Accounts() const;

	status_t			LoadTemplate(BView* parent);
	status_t			Load(const char* account, BView* parent);
	status_t			Save(const char* account, BView* parent);
	status_t			Delete(const char* account);

private:
	status_t			fStatus;
	CayaProtocol*		fProtocol;
	BString				fAccount;
	BMessage*			fTemplate;

	void				_Init();
	status_t			_Load(const char* account, BMessage** settings);
	status_t			_Save(const char* account, BMessage* settings);
};

#endif	// _PROTOCOL_SETTINGS_H
