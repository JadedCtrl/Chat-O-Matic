/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H
 
#include <Window.h>

#include "Observer.h"

class BSplitView;
class BTextView;

class Conversation;
class ConversationItem;
class ConversationListView;
class ConversationView;
class RosterItem;
class Server;
class StatusView;


class MainWindow: public BWindow, public Observer {
public:
						MainWindow();

			void		Start();
	virtual	bool		QuitRequested();

	virtual	void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);
			void		ImError(BMessage* msg);

			// Observer inheritance
			void		ObserveInteger(int32 what, int32 val);

	virtual void		WorkspaceActivated(int32 workspace,
							bool active);

			void		SetConversation(Conversation* chat);
			Server*		GetServer() const { return fServer; }


private:
			void		_InitInterface();
			BMenuBar*	_CreateMenuBar();

			ConversationItem* _EnsureConversationItem(BMessage* msg);
			void		_UpdateListItem(ConversationItem* item);	

	Server*				fServer;
	bool				fWorkspaceChanged;

	// Left panel, chat list
	ConversationListView* fListView;
	StatusView*			fStatusView;

	// Right panel, chat
	BSplitView*			fRightView;
	BScrollView*		fSendScroll;
	BTextView*			fSendView;
	ConversationView*	fChatView;
};


#endif	// _MAIN_WINDOW_H

