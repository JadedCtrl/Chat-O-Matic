/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONLIST_H
#define CONVERSATIONLIST_H

#include <OutlineListView.h>

class BPopUpMenu;
class Conversation;
class ConversationAccountItem;


class ConversationListView : public BOutlineListView {
public:
	ConversationListView(const char* name);

	void MessageReceived(BMessage* msg);
	void SelectionChanged();
	void MouseDown(BPoint where);

	void AddConversation(Conversation* chat);
	void RemoveConversation(Conversation* chat);

	int32 CountConversations();
	int32 ConversationIndexOf(Conversation* chat);
	void SelectConversation(int32 index);

private:
	BPopUpMenu* _ConversationPopUp();
	BPopUpMenu* _BlankPopUp();

	ConversationAccountItem* _EnsureAccountItem(Conversation* chat);
};


#endif // CONVERSATIONLIST_H

