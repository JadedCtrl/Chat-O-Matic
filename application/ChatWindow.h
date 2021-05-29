/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHAT_WINDOW_H
#define _CHAT_WINDOW_H

#include <Notification.h>
#include <StringView.h>
#include <TextView.h>
#include <Window.h>

#include "CayaConstants.h"
#include "Observer.h"

class BitmapView;
class BScrollVie;
class Conversation;
class ConversationView;
class CayaRenderView;
class Contact;


class ChatWindow : public BWindow, public Observer {
public:
						ChatWindow();
						ChatWindow(Conversation* cl);
			void		SetConversation(Conversation* chat);

	virtual void		ShowWindow();

	virtual	void		MessageReceived(BMessage* message);
	virtual	bool		QuitRequested();

			void		ImMessage(BMessage* msg);

			void		ObserveString(int32 what, BString str);
			void		ObservePointer(int32 what, void* ptr);
			void		ObserveInteger(int32 what, int32 val);

			void		AvoidFocus(bool avoid);
private:
		BScrollView*	fSendScroll;
		BTextView*		fSendView;
		ConversationView*	fChatView;
};

#endif	// _CHAT_WINDOW_H

