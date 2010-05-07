/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_SETTINGS_H
#define _PROTOCOL_SETTINGS_H

class BMessage;
class CayaProtocol;

class ProtocolSettings {
public:
					ProtocolSettings(CayaProtocol* cayap);
					~ProtocolSettings();

	status_t		InitCheck() const;

	BList*			Accounts() const;

	status_t		Load(const char* account);
	status_t		Save(const char* account);
	status_t		Delete(const char* account);

	void			BuildGUI(BView* parent);
	status_t		SaveGUI(BView* parent);

private:
	status_t		fStatus;
	CayaProtocol*	fProtocol;
	BString			fAccount;
	BMessage*		fTemplate;
	BMessage*		fSettings;

	void			_Init();
};

#endif	// _PROTOCOL_SETTINGS_H
