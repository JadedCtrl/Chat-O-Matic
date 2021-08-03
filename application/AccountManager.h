/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_MANAGER_H
#define _ACCOUNT_MANAGER_H

#include <Messenger.h>

#include "AppConstants.h"
#include "Notifier.h"


class AccountManager : public Notifier {
public:
	static	AccountManager*	Get();

			void			SetNickname(BString nick, int64 instance = -1);

			UserStatus		Status() const;
			void			SetStatus(UserStatus status, int64 instance = -1);
			void			SetStatus(UserStatus status, const char* str,
								int64 instance = -1);

			void			SetReplicantMessenger(BMessenger* messenger);
			void			ReplicantStatusNotify(UserStatus status,
								bool wait = false);
private:
							AccountManager();
							~AccountManager();

	UserStatus				fStatus;
	BMessenger*				fReplicantMessenger;

};

#endif	// _ACCOUNT_MANAGER_H
