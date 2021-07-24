/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_BEHAVIOR_H
#define _PREFERENCES_BEHAVIOR_H

#include <View.h>

class BCheckBox;

class PreferencesBehavior : public BView {
public:
					PreferencesBehavior();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:

	BCheckBox*		fHideOffline;
	BCheckBox*		fToCurrentWorkspace;
	BCheckBox*		fRaiseOnMessageReceived;
	BCheckBox*		fRaiseUserIsTyping;
	BCheckBox*		fPlaySoundOnMessageReceived;
	BCheckBox*		fMarkUnreadWindow;
	BCheckBox*		fMarkUnreadReplicant;

	BCheckBox*		fDisableQuitConfirm;

	BCheckBox*		fNotifyProtocols;
	BCheckBox*		fNotifyContactStatus;	
	BCheckBox*		fNotifyNewMessage;
};

#endif	// _PREFERENCES_BEHAVIOR_H
