/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Dario Casalinuovo
 */
#include "Contact.h"

#include "Utils.h"
#include "ProtocolLooper.h"
#include "RosterItem.h"


Contact::Contact(BString id, BMessenger msgn)
	:
	User::User(id, msgn)
{
	fRosterItem = new RosterItem(id.String(), this);
	RegisterObserver(fRosterItem);
}


RosterItem*
Contact::GetRosterItem() const
{
	return fRosterItem;
}


void
Contact::_EnsureCachePath()
{
	if (fCachePath.InitCheck() == B_OK)
		return;
	fCachePath.SetTo(BuddyCachePath(fLooper->Protocol()->GetName(),
										  fID.String()));
}


