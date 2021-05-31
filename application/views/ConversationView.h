/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHAT_VIEW_H
#define _CHAT_VIEW_H

#include <GroupView.h>
#include <ObjectList.h>
#include <StringView.h>
#include <TextView.h>

#include "CayaConstants.h"

class BMessageQueue;

class BitmapView;
class CayaRenderView;
class Contact;
class Conversation;


class ConversationView : public BGroupView {
public:
						ConversationView();
						ConversationView(Conversation* chat);

	virtual	void		MessageReceived(BMessage* message);
	virtual	bool		QuitRequested();

			Conversation* GetConversation();
			void		SetConversation(Conversation* chat);

			void		AttachedToWindow();

			void		UpdateAvatar();
			void		UpdatePersonalMessage();
			void		ImMessage(BMessage* msg);

			void		ObserveString(int32 what, BString str);
			void		ObservePointer(int32 what, void* ptr);
			void		ObserveInteger(int32 what, int32 val);

			void		AppendStatus(CayaStatus status);

			void		AvoidFocus(bool avoid);

private:
			bool		_AppendOrEnqueueMessage(BMessage* msg);
			void		_AppendMessage(BMessage* msg);

		Conversation*	fConversation;
		Contact*		fContact;
		CayaRenderView*	fReceiveView;
		BStringView*	fStatus;
		BTextView*		fPersonalMessage;
		BitmapView*		fProtocolView;
		BitmapView*		fAvatar;
		int32			fMessageCount;
		BObjectList<BMessage>	fMessageQueue;
};


#endif	// _CHAT_VIEW_H

