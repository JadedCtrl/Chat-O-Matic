/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACCOUNT_MANAGER_H
#define _ACCOUNT_MANAGER_H

#include "CayaConstants.h"
#include "Notifier.h"

class AccountManager : public Notifier {
public:
	static	AccountManager*	Get();

			void			SetNickname(BString nick);

			CayaStatus		Status() const;
			void			SetStatus(CayaStatus status,
								const char* str = NULL);

private:
							AccountManager();
							~AccountManager();

	CayaStatus				fStatus;
};

#endif	// _ACCOUNT_MANAGER_H
