/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_NOTIFICATIONS_H
#define _PREFERENCES_NOTIFICATIONS_H

#include <View.h>

class BButton;
class BCheckBox;

class PreferencesNotifications : public BView {
public:
					PreferencesNotifications();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:
	BCheckBox*		fNotifyProtocols;
	BCheckBox*		fNotifyContactStatus;	
	BCheckBox*		fNotifyNewMessage;
	BCheckBox*		fSoundOnMessageReceived;
	BCheckBox*		fSoundOnMention;

	BButton*		fSoundsButton;
};

#endif	// _PREFERENCES_NOTIFICATIONS_H
