/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _STATUS_MANAGER_H
#define _STATUS_MANAGER_H

#include <Messenger.h>

#include "AppConstants.h"
#include "Notifier.h"


class StatusManager : public Notifier {
public:
	static	StatusManager*	Get();

			void			SetNickname(BString nick, int64 instance = -1);

			UserStatus		Status() const;
			void			SetStatus(UserStatus status, int64 instance = -1);
			void			SetStatus(UserStatus status, const char* str,
								int64 instance = -1);

			void			SetReplicantMessenger(BMessenger* messenger);
			void			ReplicantStatusNotify(UserStatus status,
								bool wait = false);
private:
							StatusManager();
							~StatusManager();

	UserStatus				fStatus;
	BMessenger*				fReplicantMessenger;

};

#endif	// _STATUS_MANAGER_H
