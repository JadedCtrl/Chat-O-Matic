/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TEMPLATE_WINDOW_H
#define _TEMPLATE_WINDOW_H

#include <String.h>
#include <Window.h>

#include "ProtocolTemplate.h"
#include "Server.h"

class BAlert;
class BMenu;
class BMenuField;
class BTextControl;
class ProtocolSettings;
class TemplateView;


class TemplateWindow : public BWindow {
public:
						TemplateWindow(const char* title,
							const char* templateType, BMessage* msg,
							Server* server);

	virtual	void		MessageReceived(BMessage* msg);

			void		SetTarget(BHandler* target);

private:
			void		_InitInterface();
			void		_LoadTemplate();
			BMenu*		_CreateAccountMenu();

	Server*				fServer;
	AccountInstances	fAccounts;
	int32				fSelectedAcc;
	BMenuField*			fMenuField;

	ProtocolTemplate*	fTemplate;
	BString				fTemplateType;
	TemplateView*		fTemplateView;

	BButton*			fOkButton;

	BMessage*			fMessage;
	BHandler*			fTarget;
};

#endif	// _TEMPLATE_WINDOW_H
