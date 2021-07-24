/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_REPLICANT_H
#define _PREFERENCES_REPLICANT_H

#include <View.h>

class BCheckBox;

class PreferencesReplicant : public BView {
public:
					PreferencesReplicant();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:	
	BCheckBox*		fDisableReplicant;
	BCheckBox*		fPermanentReplicant;
	BCheckBox*		fHideDeskbar;

};

#endif	// _PREFERENCES_REPLICANT_H
