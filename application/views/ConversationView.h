/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHAT_VIEW_H
#define _CHAT_VIEW_H

#include <GroupView.h>
#include <ObjectList.h>

#include "AppConstants.h"
#include "Conversation.h"
#include "Observer.h"

class BStringView;
class BTextView;

class BitmapView;
class RenderView;
class SendTextView;
class User;
class UserListView;


class ConversationView : public BGroupView, public Observer, public Notifier {
public:
						ConversationView();
						ConversationView(Conversation* chat);

	virtual	bool		QuitRequested();
	virtual void		AttachedToWindow();

	virtual	void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);

			Conversation* GetConversation();
			void		SetConversation(Conversation* chat);

			void		UpdateIcon();

			void		UpdateUserList(UserMap users);
			void		InvalidateUserList();

			void		ObserveString(int32 what, BString str);

			void		AvoidFocus(bool avoid);

private:
			void		_InitInterface();

			bool		_AppendOrEnqueueMessage(BMessage* msg);
			void		_AppendMessage(BMessage* msg);

			void		_UserMessage(const char* format, const char* bodyFormat,
									 BMessage* msg);

		Conversation* fConversation;
		BObjectList<BMessage> fMessageQueue;

		BTextView* fNameTextView;
		BTextView* fSubjectTextView;
		BitmapView* fProtocolView;
		BitmapView* fIcon;

		RenderView* fReceiveView;
		UserListView* fUserList;
		SendTextView* fSendView;
};


#endif	// _CHAT_VIEW_H

