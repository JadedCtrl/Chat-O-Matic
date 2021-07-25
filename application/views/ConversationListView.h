/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _CONVERSATION_LIST_H
#define _CONVERSATION_LIST_H

#include <OutlineListView.h>

class BPopUpMenu;
class Conversation;
class ConversationAccountItem;


class ConversationListView : public BOutlineListView {
public:
	ConversationListView(const char* name);

	virtual void	MessageReceived(BMessage* msg);
	virtual	void	SelectionChanged();
	virtual void	MouseDown(BPoint where);

			void	AddConversation(Conversation* chat);
			void	RemoveConversation(Conversation* chat);
			void	SortConversation(Conversation* chat);

			int32	CountConversations();
			int32	ConversationIndexOf(Conversation* chat);
			void	SelectConversation(int32 index);

private:
	  BPopUpMenu*	_ConversationPopUp();
	  BPopUpMenu*	_BlankPopUp();

	ConversationAccountItem*
					_EnsureAccountItem(Conversation* chat);
};

#endif // _CONVERSATION_LIST_H
