/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_LIST_ITEM_H
#define _ACCOUNT_LIST_ITEM_H

#include <String.h>
#include <StringItem.h>

class ProtocolSettings;

class AccountListItem : public BStringItem {
public:
								AccountListItem(ProtocolSettings* settings,
								                const char* account);
	virtual						~AccountListItem();

			ProtocolSettings*	Settings() const;

			const char*			Account() const;
			void				SetAccount(const char* name);

private:
			ProtocolSettings*	fSettings;
			BString				fAccount;
			float				fBaselineOffset;
};

#endif	// _ACCOUNT_LIST_ITEM_H
