/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Dario Casalinuovo. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_CHATWINDOW_H
#define _PREFERENCES_CHATWINDOW_H

#include <View.h>

class BCheckBox;
class BStringView;

class PreferencesChatWindow : public BView {
public:
					PreferencesChatWindow();

	virtual	void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage* msg);

private:

	BStringView*	fChatWindowString;

	BCheckBox*		fIgnoreEmoticons;
};

#endif	// _PREFERENCES_BEHAVIOR_H
