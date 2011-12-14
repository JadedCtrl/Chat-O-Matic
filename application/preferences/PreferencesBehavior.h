/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_BEHAVIOR_H
#define _PREFERENCES_BEHAVIOR_H

#include <View.h>

class BCheckBox;
class BStringView;

class PreferencesBehavior : public BView {
public:
					PreferencesBehavior();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:

	BStringView*	fOnIncoming;
	BCheckBox*		fToCurrentWorkspace;
	BCheckBox*		fActivateChatWindow;
	BCheckBox*		fPlaySoundOnMessageReceived;
	BCheckBox*		fMarkUnreadWindow;

	BCheckBox*		fIgnoreEmoticons;

	BCheckBox*		fDisableReplicant;
	BCheckBox*		fPermanentReplicant;
	BCheckBox*		fHideCayaTracker;

};

#endif	// _PREFERENCES_BEHAVIOR_H
