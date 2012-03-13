/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_REPLICANT_H
#define _PREFERENCES_REPLICANT_H

#include <View.h>

class BCheckBox;
class BStringView;

class PreferencesReplicant : public BView {
public:
					PreferencesReplicant();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:	
	BStringView*	fReplicantString;
	BCheckBox*		fDisableReplicant;
	BCheckBox*		fPermanentReplicant;
	BCheckBox*		fHideCayaDeskbar;

};

#endif	// _PREFERENCES_REPLICANT_H
