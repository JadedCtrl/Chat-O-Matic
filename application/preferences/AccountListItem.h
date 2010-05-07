/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_LIST_ITEM_H
#define _ACCOUNT_LIST_ITEM_H

#include <String.h>
#include <StringItem.h>

class CayaProtocol;
class ProtocolSettings;

class AccountListItem : public BStringItem {
public:
								AccountListItem(CayaProtocol* cayap,
								                const char* account);
	virtual						~AccountListItem();

			ProtocolSettings*	Settings() const;
			CayaProtocol*		Protocol() const;
			const char*			Account() const;

			void				DrawItem(BView* owner, BRect frame,
									     bool complete = false);

			void				Update(BView* owner, const BFont* font);

private:
			CayaProtocol*		fProtocol;
			BString				fAccount;
			ProtocolSettings*	fSettings;
			float				fBaselineOffset;
};

#endif	// _ACCOUNT_LIST_ITEM_H
