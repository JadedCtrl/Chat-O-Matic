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
class BView;
class CayaProtocolAddOn;


class ProtocolTemplate {
public:
						ProtocolTemplate(CayaProtocolAddOn* addOn,
							const char* type);
						~ProtocolTemplate();

	status_t			InitCheck() const;
	CayaProtocolAddOn*	AddOn() const;

	status_t			Load(BView* parent, BMessage* settings = NULL);
	BMessage*			Save(BView* parent);

private:
	CayaProtocolAddOn*	fAddOn;
	BMessage*			fTemplate;
};

#endif	// _PROTOCOL_TEMPLATE_H
