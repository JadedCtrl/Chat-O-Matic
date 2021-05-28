/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHAT_VIEW_H
#define _CHAT_VIEW_H

#include <Window.h>
#include <TextView.h>
#include <StringView.h>
#include <GroupView.h>
#include <Notification.h>
#include "Observer.h"

#include "CayaConstants.h"

class BitmapView;
class Conversation;
class CayaRenderView;
class Contact;


class ConversationView : public BGroupView {
public:
						ConversationView(Conversation* chat);

//	virtual void		ShowWindow();

	virtual	void		MessageReceived(BMessage* message);
//			void		WindowActivated(bool active);
	virtual	bool		QuitRequested();

			Conversation* GetConversation();

			void		UpdateAvatar();
			void		UpdatePersonalMessage();
			void		ImMessage(BMessage* msg);

			void		ObserveString(int32 what, BString str);
			void		ObservePointer(int32 what, void* ptr);
			void		ObserveInteger(int32 what, int32 val);
			void		AppendStatus(CayaStatus status);

			void		AvoidFocus(bool avoid);
private:
		Conversation*	fConversation;
		Contact*		fContact;
		CayaRenderView*	fReceiveView;
		BStringView*	fStatus;
		BTextView*		fPersonalMessage;
		BitmapView*		fProtocolView;
		BitmapView*		fAvatar;
		int32			fMessageCount;

};


#endif	// _CHAT_VIEW_H

