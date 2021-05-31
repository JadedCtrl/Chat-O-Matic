/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CHAT_VIEW_H
#define _CHAT_VIEW_H

#include <GroupView.h>
#include <ObjectList.h>

#include "CayaConstants.h"
#include "Conversation.h"

class BStringView;
class BTextView;

class BitmapView;
class CayaRenderView;
class Contact;
class UserListView;


class ConversationView : public BGroupView {
public:
						ConversationView();
						ConversationView(Conversation* chat);

	virtual	bool		QuitRequested();
	virtual void		AttachedToWindow();

	virtual	void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);

			Conversation* GetConversation();
			void		SetConversation(Conversation* chat);


			void		UpdateAvatar();
			void		UpdatePersonalMessage();
			void		UpdateUserList(UserMap users);

			void		ObserveString(int32 what, BString str);
			void		ObservePointer(int32 what, void* ptr);
			void		ObserveInteger(int32 what, int32 val);

			void		AppendStatus(CayaStatus status);

			void		AvoidFocus(bool avoid);

private:
			void		_InitInterface();

			bool		_AppendOrEnqueueMessage(BMessage* msg);
			void		_AppendMessage(BMessage* msg);

		Conversation*	fConversation;
		Contact*		fContact;
		int32			fMessageCount;
		BObjectList<BMessage>	fMessageQueue;

		CayaRenderView*	fReceiveView;
		BStringView*	fStatus;
		BTextView*		fPersonalMessage;
		BitmapView*		fProtocolView;
		BitmapView*		fAvatar;
		UserListView*	fUserList;
};


#endif	// _CHAT_VIEW_H

