/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONLIST_H
#define CONVERSATIONLIST_H

#include <OutlineListView.h>

class BPopUpMenu;


class ConversationListView : public BListView {
public:
	ConversationListView(const char* name);

	void MessageReceived(BMessage* msg);
	void MouseDown(BPoint where);

private:
	BPopUpMenu* _ConversationPopUp();
	BPopUpMenu* _BlankPopUp();
};


#endif // CONVERSATIONLIST_H

