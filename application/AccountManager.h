/*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_MANAGER_H
#define _ACCOUNT_MANAGER_H

#include <Messenger.h>

#include "CayaConstants.h"
#include "Notifier.h"


class AccountManager : public Notifier {
public:
	static	AccountManager*	Get();

			void			SetNickname(BString nick);

			CayaStatus		Status() const;
			void			SetStatus(CayaStatus status,
								const char* str = NULL);

			void			SetReplicantMessenger(BMessenger* messenger);
private:
							AccountManager();
							~AccountManager();

			void			_ReplicantStatusNotify(CayaStatus status);

	CayaStatus				fStatus;
	BMessenger*				fReplicantMessenger;

};

#endif	// _ACCOUNT_MANAGER_H
