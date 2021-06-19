/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PROTOCOL_TEMPLATE_H
#define _PROTOCOL_TEMPLATE_H

#include <SupportDefs.h>

class BMessage;
class BString;
class BView;
class CayaProtocol;


class ProtocolTemplate {
public:
						ProtocolTemplate(CayaProtocol* protocol,
							const char* type);
						ProtocolTemplate(BMessage pTemplate);
						~ProtocolTemplate();

	status_t			InitCheck() const;
	CayaProtocol*		Protocol() const;

	status_t			Load(BView* parent, BMessage* settings = NULL);
	status_t			Save(BView* parent, BMessage* settings,
							BString* errorText = NULL);

private:
	CayaProtocol*		fProtocol;
	BMessage*			fTemplate;
};

#endif	// _PROTOCOL_TEMPLATE_H
