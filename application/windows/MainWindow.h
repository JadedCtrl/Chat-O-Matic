/*
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _MAIN_WINDOW_H
#define _MAIN_WINDOW_H
 
#include <Window.h>

#include "Server.h"

class BMenu;
class BSplitView;
class BTextView;

class Conversation;
class ConversationItem;
class ConversationListView;
class ConversationView;
class ProtocolSettings;
class RosterItem;
class RosterEditWindow;
class RosterWindow;
class Server;
class StatusView;


class MainWindow: public BWindow {
public:
						MainWindow();

			void		Start();
	virtual	bool		QuitRequested();

	virtual	void		MessageReceived(BMessage* message);
			void		ImMessage(BMessage* msg);

	virtual void		WorkspaceActivated(int32 workspace,
							bool active);

			void		SetConversation(Conversation* chat);
			void		RemoveConversation(Conversation* chat);
			void		SortConversation(Conversation* chat);

			Server*		GetServer() const { return fServer; }

private:
			void		_InitInterface();

			BMenuBar*	_CreateMenuBar();
			BMenu*		_CreateAccountsMenu();
			void		_RefreshAccountsMenu();

			void		_ToggleMenuItems();

			ConversationItem*
						_EnsureConversationItem(BMessage* msg);
	
			bool		_PopulateWithAccounts(BMenu* menu,
							ProtocolSettings* settings);
			void		_ReplaceMenu(const char* name, BMenu* newMenu);

	Server*				fServer;
	RosterWindow*		fRosterWindow;
	RosterEditWindow*	fRosterEditWindow;
	bool				fWorkspaceChanged;
	BMenuBar*			fMenuBar;

	// Left panel, chat list
	ConversationListView* fListView;
	StatusView*			fStatusView;
	BSplitView*			fSplitView;

	// Right panel, chat
	BSplitView*			fRightView;
	ConversationView*	fChatView;
	Conversation*		fConversation;
};


#endif	// _MAIN_WINDOW_H

