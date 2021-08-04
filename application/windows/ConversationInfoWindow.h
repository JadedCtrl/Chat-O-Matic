/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CONVERSATION_INFO_WINDOW_H
#define _CONVERSATION_INFO_WINDOW_H

#include <Window.h>

#include "Observer.h"

class BStringView;
class BTextView;

class BitmapView;
class Conversation;
class UrlTextView;


class ConversationInfoWindow : public BWindow, public Observer {
public:
					ConversationInfoWindow(Conversation* chat);
					~ConversationInfoWindow();

	virtual void	ObserveString(int32 what, BString string);
	virtual void	ObserveInteger(int32 what, int32 num);
	virtual void	ObservePointer(int32 what, void* ptr);

private:
			void	_InitInterface();

			void	_SetIdLabel(BString id);
			void	_SetUserCountLabel(int32 userCount);

	Conversation*	fChat;

	BStringView*	fNameLabel;
	BStringView*	fUserCountLabel;
	BTextView*		fIdLabel;
	BitmapView*		fIcon;
};

#endif // _CONVERSATION_INFO_WINDOW_H
