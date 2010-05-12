/*
 * Copyright 2010, Alexander Botero-Lowry. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _AIM_H
#define _AIM_H

#include <List.h>
#include <Messenger.h>
#include <String.h>

#include "CayaProtocol.h"
#include "CayaConstants.h"

#include "imcomm.h"

class AIMProtocol : public CayaProtocol {
public:
							AIMProtocol();
	virtual					~AIMProtocol();

	virtual	status_t		Init(CayaProtocolMessengerInterface* messenger);

	virtual	status_t		Shutdown();
	virtual	status_t		Process(BMessage*);

	virtual	const char*		GetSignature();
	virtual	const char*		GetFriendlySignature();
	virtual	status_t		UpdateSettings(BMessage&);
	virtual	uint32			GetEncoding();

	static	int32			WaitForData(void*);

	static	void			GotMessage(void*, char*, int, char*);
	static	void			BuddyOnline(void*, char*);
	static	void			BuddyOffline(void*, char*);
	static	void			BuddyAway(void*, char*);
	static	void			BuddyBack(void*, char*);
	static	void			BuddyAwayMsg(void*, char*, char*);
	static	void			BuddyIdle(void*, char*, long);
	static	void			BuddyProfile(void*, char*, char*);

protected:
			status_t		A_LogOn();
			status_t		LogOff();
			void			UnsupportedOperation();

	static	char*			strip_html(const char* message);

private:
			BString			fUsername;
			BString			fPassword;
			bool			fOnline;
			thread_id		fIMCommThread;
			void*			fIMCommHandle;
};

#endif	// _AIM_H
