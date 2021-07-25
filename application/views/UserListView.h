/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef CONVERSATIONLIST_H
#define CONVERSATIONLIST_H

#include <ListView.h>

#include "Role.h"

class BPopUpMenu;
class Conversation;


enum
{
	kUserInfo = 'ULui',
	kDeafenUser = 'UMdu',
	kUndeafenUser = 'UMud',
	kMuteUser = 'UMmu',
	kUnmuteUser = 'UMum',
	kKickUser = 'UMku',
	kBanUser = 'UMbu'
};


class UserListView : public BListView {
public:
					UserListView(const char* name);

	virtual void	MouseDown(BPoint where);

			void	Sort();

			void	SetConversation(Conversation* chat) { fChat = chat; }

private:
	 BPopUpMenu*	_UserPopUp();
	 BPopUpMenu*	_BlankPopUp();

			void	_ModerationAction(int32 im_what);
			void	_ProcessItem(BMessage* itemMsg, BPopUpMenu* menu,
						Role* user, Role* target, BString target_id);

	Conversation* fChat;
};

#endif // CONVERSATIONLIST_H
