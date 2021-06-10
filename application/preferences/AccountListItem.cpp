/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <View.h>

#include "AccountListItem.h"
#include "ProtocolSettings.h"


AccountListItem::AccountListItem(ProtocolSettings* settings,
	const char* account)
	:
	BStringItem(account),
	fSettings(settings),
	fAccount(account),
	fBaselineOffset(0)
{
}


AccountListItem::~AccountListItem()
{
}


ProtocolSettings*
AccountListItem::Settings() const
{
	return fSettings;
}


const char*
AccountListItem::Account() const
{
	return fAccount.String();
}


void
AccountListItem::SetAccount(const char* name)
{
	fAccount = name;
	SetText(fAccount);
}


