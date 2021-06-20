/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CONTACT_LINKER_H_
#define _CONTACT_LINKER_H_

#include <String.h>
#include <Message.h>
#include <Messenger.h>
#include <Path.h>

#include "AppConstants.h"
#include "User.h"

class BBitmap;

class ProtocolLooper;
class RosterItem;


class Contact : public User {
public:
					Contact(BString id, BMessenger msgn);

	RosterItem*		GetRosterItem() const;

	void			SetNotifyAvatarBitmap(BBitmap* bitmap);

private:
	virtual void	_EnsureCachePath();

	RosterItem*		fRosterItem;
};

#endif	// _CONTACT_LINKER_H_
