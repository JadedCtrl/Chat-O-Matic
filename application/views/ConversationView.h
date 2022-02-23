/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2021-2022, Jaidyn Levesque. All rights reserved.
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
class BSplitView;

class BitmapView;
class EnterTextView;
class RenderView;
class SendTextView;
class User;
class UserListView;


const uint32 kClearText = 'CVct';


class ConversationView : public BGroupView, public Observer, public Notifier {
public:
						ConversationView(Conversation* chat = NULL);

	virtual void		AttachedToWindow();
			void		Show();

	virtual	void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);

			Conversation* GetConversation();
			void		SetConversation(Conversation* chat);

			void		UpdateUserList(UserMap users);
			void		InvalidateUserList();

			void		ObserveString(int32 what, BString str);
			void		ObservePointer(int32 what, void* ptr);

			void		GetWeights(float* horizChat, float* horizList,
							float* vertChat, float* vertSend);
			void		SetWeights(float horizChat, float horizList,
							float vertChat, float vertSend);

private:
	typedef KeyMap<uint16, int32> UInt16IntMap;

			void		_InitInterface();

			bool		_AppendOrEnqueueMessage(BMessage* msg);
			void		_AppendMessage(BMessage* msg);

			void		_ScrollToBottom();

			// Helper functions for _AppendFormattedMessage()
			void		_EnableStartingFaces(BMessage* msg, int32 index,
							uint16* face, UInt16IntMap* indices, int32* next);
			void		_DisableEndingFaces(BMessage* msg, int32 index,
							uint16* face, UInt16IntMap* indices);
			void		_EnableStartingColor(BMessage* msg, int32 index,
							rgb_color* color, int32* indice, int32* next);

			void		_UserMessage(const char* format, const char* bodyFormat,
									 BMessage* msg);

			// When the user hasn't joined any real conversations
			void		_FakeChat();

		Conversation* fConversation;
		BObjectList<BMessage> fMessageQueue;

		EnterTextView* fNameTextView;
		EnterTextView* fSubjectTextView;
		BitmapView* fProtocolView;
		BitmapView* fIcon;

		RenderView* fReceiveView;
		UserListView* fUserList;
		SendTextView* fSendView;
		BSplitView* fHorizSplit;
		BSplitView* fVertSplit;
};

#endif	// _CHAT_VIEW_H
