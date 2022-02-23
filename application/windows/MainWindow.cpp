/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021-2022, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 *		Humdinger, humdingerb@gmail.com
 */

#include <Application.h>
#include <Alert.h>
#include <Beep.h>
#include <CardLayout.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <PathFinder.h>
#include <Roster.h>
#include <ScrollView.h>
#include <TranslationUtils.h>

#include "AccountDialog.h"
#include "AccountsWindow.h"
#include "AppMessages.h"
#include "AppPreferences.h"
#include "ChatProtocolAddOn.h"
#include "ChatProtocolMessages.h"
#include "ConversationItem.h"
#include "ConversationListView.h"
#include "ConversationView.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "PreferencesWindow.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"
#include "ReplicantStatusView.h"
#include "RoomListWindow.h"
#include "RosterEditWindow.h"
#include "RosterWindow.h"
#include "Server.h"
#include "StatusManager.h"
#include "StatusView.h"
#include "TemplateWindow.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


const uint32 kLogin			= 'LOGI';


MainWindow::MainWindow()
	:
	BWindow(AppPreferences::Get()->MainWindowRect,
		B_TRANSLATE_SYSTEM_NAME(APP_NAME), B_TITLED_WINDOW, 0),
	fWorkspaceChanged(false),
	fConversation(NULL),
	fRosterWindow(NULL)
{
	// Filter messages using Server
	AddFilter(Server::Get());

	_InitInterface();

	add_system_beep_event(APP_MENTION_BEEP);
	add_system_beep_event(APP_MESSAGE_BEEP);

	//TODO check for errors here
	ReplicantStatusView::InstallReplicant();
}


void
MainWindow::Start()
{
	// No accounts, show account window
	if (ProtocolManager::Get()->CountProtocolInstances() == 0)
		MessageReceived(new BMessage(APP_SHOW_ACCOUNTS));

	// Login all accounts
	Server::Get()->LoginAll();
}


void
MainWindow::Show()
{
	if (fChatLayout->CountItems() == 0)
		SetConversation(NULL);
	BWindow::Show();
}


bool
MainWindow::QuitRequested()
{
	int32 button_index = 0;
	if(!AppPreferences::Get()->DisableQuitConfirm)
	{
		BAlert* alert = new BAlert(B_TRANSLATE("Closing"),
			B_TRANSLATE("Are you sure you want to quit?"),
			B_TRANSLATE("Yes"), B_TRANSLATE("No"), NULL, B_WIDTH_AS_USUAL,
			B_OFFSET_SPACING, B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		button_index = alert->Go();
	}

	AppPreferences::Get()->MainWindowRect = Frame();
	_SaveWeights();

	if (button_index == 0) {
		Server::Get()->Quit();

		// ConversationViews will be removed by Server's deletion of Conversations,
		// but some special views (protocol logs, the blank ConversationView)
		// must be done manually
		for (int i = fChatLayout->CountItems() - 1; i >= 0; i--)
			fChatLayout->RemoveItem(i);
		fChatLayout->RemoveSelf();
		delete fChatLayout;

		AppPreferences::Get()->Save();
		ReplicantStatusView::RemoveReplicant();
		be_app->PostMessage(B_QUIT_REQUESTED);
		return true;
	} else
		return false;
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case APP_SHOW_SETTINGS:
		{
			PreferencesWindow* win = new PreferencesWindow();
			win->Show();
			break;
		}
		case APP_SHOW_ACCOUNTS:
		{
			AccountsWindow* win = new AccountsWindow();
			win->Show();
			break;
		}
		case APP_EDIT_ACCOUNT:
		{
			void* settings = NULL;
			BString account = message->FindString("account");
			message->FindPointer("settings", &settings);

			if (account.IsEmpty() == false && settings != NULL) {
				AccountDialog* win = new AccountDialog("Editing account",
					(ProtocolSettings*)settings, account.String());
				win->Show();
			}
			break;
		}
		case APP_TOGGLE_ACCOUNT:
		{
			ProtocolManager* protoMan = ProtocolManager::Get();
			ProtocolSettings* settings = NULL;
			BString account = message->FindString("account");
			int64 instance = message->GetInt64("instance", -1);
			message->FindPointer("settings", (void**)&settings);

			if (account.IsEmpty() == false && settings != NULL)
				protoMan->ToggleAccount(settings, account);
			break;
		}
		case APP_NEW_CHAT:
		{
			BMessage* newMsg = new BMessage(IM_MESSAGE);
			newMsg->AddInt32("im_what", IM_CREATE_CHAT);

			fRosterWindow = new RosterWindow(B_TRANSLATE("Invite contact to "
				"chat" B_UTF8_ELLIPSIS), newMsg, new BMessenger(this));
			fRosterWindow->Show();
			break;
		}
		case APP_NEW_ROOM:
		{
			BMessage* createMsg = new BMessage(IM_MESSAGE);
			createMsg->AddInt32("im_what", IM_CREATE_ROOM);

			TemplateWindow* win = new TemplateWindow(B_TRANSLATE("Create room"),
				"create_room", createMsg);
			win->Show();
			break;
		}
		case APP_JOIN_ROOM:
		{
			BMessage* joinMsg = new BMessage(IM_MESSAGE);
			joinMsg->AddInt32("im_what", IM_JOIN_ROOM);

			TemplateWindow* win = new TemplateWindow(B_TRANSLATE("Join a room"),
				"join_room", joinMsg);
			win->Show();
			break;
		}
		case APP_SEND_INVITE:
		{
			if (fConversation == NULL)
				break;
			BString chat_id = fConversation->GetId();

			BMessage* invite = new BMessage(IM_MESSAGE);
			invite->AddInt32("im_what", IM_ROOM_SEND_INVITE);
			invite->AddString("chat_id", chat_id);

			ProtocolLooper* plooper = fConversation->GetProtocolLooper();
			BLooper* looper = (BLooper*)plooper;
			fRosterWindow = new RosterWindow(B_TRANSLATE("Invite contact to "
				"chat" B_UTF8_ELLIPSIS), invite, new BMessenger(looper),
				plooper->GetInstance());

			fRosterWindow->Show();
			break;
		}
		case APP_ROOM_DIRECTORY:
		{
			RoomListWindow::Get()->Show();
			break;
		}
		case APP_ROOM_SEARCH:
		{
			if (fConversation != NULL) {
				entry_ref ref;
				BEntry entry(fConversation->CachePath().Path());
				if (entry.GetRef(&ref) != B_OK)
					break;

				BMessage msg(B_REFS_RECEIVED);
				msg.AddRef("refs", &ref);
				BRoster roster;
				roster.Launch("application/x-vnd.Haiku.TextSearch", &msg);
			}
			break;
		}
		case APP_EDIT_ROSTER:
		{
			RosterEditWindow::Get()->Show();
			break;
		}
		case APP_MOVE_UP:
		{
			int32 index = fListView->CurrentSelection();
			if (index > 0)
				fListView->Select(index - 1);
			break;
		}
		case APP_MOVE_DOWN:
		{
			int32 index = fListView->CurrentSelection();
			int32 count = fListView->CountItems();
			if (index < (count - 1))
				fListView->Select(index + 1);
			break;
		}
		case APP_REPLICANT_STATUS_SET:
		{
			int32 status;
			message->FindInt32("status", &status);
			StatusManager* statusMan = StatusManager::Get();
			statusMan->SetStatus((UserStatus)status);
			break;
		}
		case APP_REPLICANT_SHOW_WINDOW:
		{
			if (LockLooper()) {
				SetWorkspaces(B_CURRENT_WORKSPACE);
				
				if ((IsMinimized() || IsHidden()) 
					|| fWorkspaceChanged) {
					Minimize(false);
					Show();
					fWorkspaceChanged = false;
				} else if ((!IsMinimized() || !IsHidden())
					|| (!fWorkspaceChanged)) {
					Minimize(true);
				}
				UnlockLooper();
			}
			break;
		}
		case APP_ACCOUNT_DISABLED: {
			_ToggleMenuItems();
			_RefreshAccountsMenu();
			fListView->RemoveAccount(message->GetInt64("instance", -1));
			break;
		}
		case APP_SHOW_HELP:
		{
			// Borrowed from HaikuArchives/Calendar's _ShowHelp()
			BPathFinder pathFinder;
			BStringList paths;
			BPath path;
			BEntry entry;

			status_t error = pathFinder.FindPaths(B_FIND_PATH_DOCUMENTATION_DIRECTORY,
				"packages/chat_o_matic", paths);

			for (int i = 0; i < paths.CountStrings(); ++i) {
				if (error == B_OK && path.SetTo(paths.StringAt(i)) == B_OK
					&& path.Append("Documentation.html") == B_OK)
				{
					entry = path.Path();
					entry_ref ref;
					entry.GetRef(&ref);
					be_roster->Launch(&ref);
				}
			}
			break;
		}
		case IM_MESSAGE:
			ImMessage(message);
			break;
		case B_ABOUT_REQUESTED:
			be_app->PostMessage(message);
			break;
		default:
			BWindow::MessageReceived(message);
	}
}


void
MainWindow::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");
	switch (im_what) {
		case IM_OWN_CONTACT_INFO:
		{
			int64 instance;
			if (msg->FindInt64("instance", &instance) == B_OK) {
				ProtocolLooper* looper = Server::Get()->GetProtocolLooper(instance);
				if (looper != NULL) {
					Contact* contact = looper->GetOwnContact();
					contact->RegisterObserver(fStatusView);
				}
			}
			break;
		}
		case IM_ROOM_JOINED:
		case IM_ROOM_PARTICIPANTS:
		case IM_ROOM_CREATED:
		case IM_CHAT_CREATED:
		case IM_MESSAGE_RECEIVED:
		case IM_MESSAGE_SENT:
		{
			_EnsureConversationItem(msg);
			break;
		}
		case IM_ROOM_LEFT:
		{
			ConversationItem* item = _EnsureConversationItem(msg);
			if (item == NULL)
				break;

			delete item->GetConversation();
			break;
		}
		case IM_USER_AVATAR_SET:
		case IM_USER_STATUS_SET:
		case IM_CONTACT_INFO:
		case IM_EXTENDED_CONTACT_INFO:
		case IM_ROSTER_CONTACT_REMOVED: {
			if (fRosterWindow != NULL)
				fRosterWindow->PostMessage(msg);
			if (RosterEditWindow::Check() == true)
				RosterEditWindow::Get()->PostMessage(msg);
			break;
		}
		case IM_PROTOCOL_READY: {
			if (fConversation == NULL)
				fBackupChatView->MessageReceived(msg);
			fStatusView->MessageReceived(msg);
			_ToggleMenuItems();
			_RefreshAccountsMenu();
			fListView->AddAccount(msg->GetInt64("instance", -1));
			break;
		}
		case IM_ROOM_DIRECTORY:
			if (RoomListWindow::Check() == true)
				RoomListWindow::Get()->PostMessage(msg);
			break;
		case IM_PROTOCOL_DISABLE:
			fStatusView->MessageReceived(msg);
			break;
	}
}


void
MainWindow::WorkspaceActivated(int32 workspace, bool active)
{
	if (active)
		fWorkspaceChanged = false;
	else
		fWorkspaceChanged = true;
}


void
MainWindow::SetConversation(Conversation* chat)
{
	if (chat == NULL) {
		SetTitle(APP_NAME);
		SetConversationView(fBackupChatView);
		return;
	}

	_EnsureConversationView(chat);

	bool found = false;
	BLayoutItem* item = fChatList.ValueFor(chat, &found);
	if (found == false)
		return;

	BString title(chat->GetName());
	title << " ― " << APP_NAME;
	SetTitle(title.String());

	// Remove "Protocol" menu
	BMenuItem* chatMenuItem = fMenuBar->FindItem(B_TRANSLATE("Chat"));
	BMenuItem* protocolMenuItem = fMenuBar->FindItem(B_TRANSLATE("Protocol"));
	if (protocolMenuItem != NULL)
		fMenuBar->RemoveItem(protocolMenuItem);

	// Populate new "Protocol" menu if need be
	BMenu* protocolMenu = _CreateProtocolMenu();
	if (protocolMenu != NULL)
		fMenuBar->AddItem(protocolMenu, fMenuBar->IndexOf(chatMenuItem) + 1);

	_SaveWeights();
	fConversation = chat;
	fChatLayout->SetVisibleItem(item);
	_ApplyWeights();
}


void
MainWindow::SetConversationView(ConversationView* view)
{
	int32 index = fChatLayout->IndexOfView(view);
	if (index < 0) {
		BLayoutItem* item = fChatLayout->AddView(view);
		fChatLayout->SetVisibleItem(item);
	} else
		fChatLayout->SetVisibleItem(index);

	_ApplyWeights();
}


void
MainWindow::RemoveConversation(Conversation* chat)
{
	SetConversation(NULL);

	bool found = false;
	BLayoutItem* item = fChatList.ValueFor(chat, &found);
	if (item != NULL)
		item->RemoveSelf();
	if (found == true)
		fChatList.RemoveItemFor(chat);

	int32 index = fListView->IndexOf(chat->GetListItem());
	if (index > 0)
		index--;

	fListView->RemoveConversation(chat);

	if (fListView->CountItems() > 0)
		fListView->Select(index);
	_ToggleMenuItems();
}


void
MainWindow::SortConversation(Conversation* chat)
{
	fListView->SortConversation(chat);
}


void
MainWindow::_InitInterface()
{
	// Left side of window, Roomlist + Status
	fListView = new ConversationListView("roomList");
	fStatusView = new StatusView("statusView");
	fSplitView = new BSplitView(B_HORIZONTAL, 0);

	BScrollView* listScroll = new BScrollView("roomListScroll", fListView,
		true, false, B_NO_BORDER);

	// Right-side of window, Chat + Textbox
	fChatLayout = new BCardLayout();
	fBackupChatView = new ConversationView();

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add((fMenuBar = _CreateMenuBar()))
		.AddGroup(B_HORIZONTAL)
			.SetInsets(5, 5, 0, 10)
			.AddSplit(fSplitView)
				.AddGroup(B_VERTICAL)
					.Add(listScroll, 1)
					.Add(fStatusView)
				.End()
				.Add(fChatLayout, 5)
			.End()
		.End()
	.End();

	_ToggleMenuItems();
}


BMenuBar*
MainWindow::_CreateMenuBar()
{
	BMenuBar* menuBar = new BMenuBar("MenuBar");

	// Program
	BMenu* programMenu = new BMenu(B_TRANSLATE("Program"));
	programMenu->AddItem(new BMenuItem(B_TRANSLATE("About" B_UTF8_ELLIPSIS),
		new BMessage(B_ABOUT_REQUESTED)));
	programMenu->AddItem(new BMenuItem(B_TRANSLATE("Help" B_UTF8_ELLIPSIS),
		new BMessage(APP_SHOW_HELP)));
	programMenu->AddItem(new BMenuItem(B_TRANSLATE("Preferences" B_UTF8_ELLIPSIS),
		new BMessage(APP_SHOW_SETTINGS), ',', B_COMMAND_KEY));
	programMenu->AddItem(new BSeparatorItem());
	programMenu->AddItem(new BMenuItem(B_TRANSLATE("Quit"),
		new BMessage(B_QUIT_REQUESTED), 'Q', B_COMMAND_KEY));
	programMenu->SetTargetForItems(this);

	// Chat
	BMenu* chatMenu = new BMenu(B_TRANSLATE("Chat"));
	chatMenu->AddItem(new BMenuItem(B_TRANSLATE("Join room" B_UTF8_ELLIPSIS),
		new BMessage(APP_JOIN_ROOM), 'J', B_COMMAND_KEY));
	chatMenu->AddItem(new BMenuItem(B_TRANSLATE("Room directory" B_UTF8_ELLIPSIS),
		new BMessage(APP_ROOM_DIRECTORY)));
	chatMenu->SetTargetForItems(this);
	chatMenu->AddSeparatorItem();
	chatMenu->AddItem(new BMenuItem(B_TRANSLATE("New room" B_UTF8_ELLIPSIS),
		new BMessage(APP_NEW_ROOM), 'N', B_COMMAND_KEY));
	chatMenu->AddItem(new BMenuItem(B_TRANSLATE("New chat" B_UTF8_ELLIPSIS),
		new BMessage(APP_NEW_CHAT), 'M', B_COMMAND_KEY));
	chatMenu->AddSeparatorItem();
	chatMenu->AddItem(new BMenuItem(B_TRANSLATE("Find" B_UTF8_ELLIPSIS),
		new BMessage(APP_ROOM_SEARCH), 'F', B_COMMAND_KEY));

	// Roster
	BMenu* rosterMenu = new BMenu(B_TRANSLATE("Roster"));
	rosterMenu->AddItem(new BMenuItem(B_TRANSLATE("Edit roster" B_UTF8_ELLIPSIS),
		new BMessage(APP_EDIT_ROSTER), 'R', B_COMMAND_KEY));
	rosterMenu->AddSeparatorItem();
	rosterMenu->AddItem(new BMenuItem(B_TRANSLATE("Invite user" B_UTF8_ELLIPSIS),
		new BMessage(APP_SEND_INVITE), 'I', B_COMMAND_KEY));
	rosterMenu->SetTargetForItems(this);

	// Window
	BMenu* windowMenu = new BMenu(B_TRANSLATE("Window"));
	windowMenu->AddItem(new BMenuItem(B_TRANSLATE("Up"),
		new BMessage(APP_MOVE_UP), B_UP_ARROW, B_COMMAND_KEY));
	windowMenu->AddItem(new BMenuItem(B_TRANSLATE("Down"),
		new BMessage(APP_MOVE_DOWN), B_DOWN_ARROW, B_COMMAND_KEY));
	windowMenu->SetTargetForItems(this);

	menuBar->AddItem(programMenu);
	menuBar->AddItem(_CreateAccountsMenu());
	menuBar->AddItem(chatMenu);
	menuBar->AddItem(rosterMenu);
	menuBar->AddItem(windowMenu);

	return menuBar;
}


BMenu*
MainWindow::_CreateAccountsMenu()
{
	BMenu* accountsMenu = new BMenu(B_TRANSLATE("Accounts"));
	ProtocolManager* pm = ProtocolManager::Get();

	bool hasAccounts = false;
	for (uint32 i = 0; i < pm->CountProtocolAddOns(); i++) {
		ChatProtocolAddOn* addOn = pm->ProtocolAddOnAt(i);
		ProtocolSettings* settings = new ProtocolSettings(addOn);

		if (_PopulateWithAccounts(accountsMenu, settings) == true)
			hasAccounts = true;
	}

	if (hasAccounts == true)
		accountsMenu->AddSeparatorItem();
	accountsMenu->AddItem(
		new BMenuItem(B_TRANSLATE("Manage accounts" B_UTF8_ELLIPSIS),
		new BMessage(APP_SHOW_ACCOUNTS), '.', B_COMMAND_KEY));
	accountsMenu->SetTargetForItems(this);
	return accountsMenu;
}


void
MainWindow::_RefreshAccountsMenu()
{
	_ReplaceMenu(B_TRANSLATE("Accounts"), _CreateAccountsMenu());
}


BMenu*
MainWindow::_CreateProtocolMenu()
{
	if (fConversation == NULL)
		return NULL;

	ProtocolLooper* looper = fConversation->GetProtocolLooper();
	BObjectList<BMessage> menuItems = looper->Protocol()->MenuBarItems();
	if (menuItems.CountItems() == 0)
		return NULL;

	BMenu* menu = new BMenu(B_TRANSLATE("Protocol"));
	for (int i = 0; i < menuItems.CountItems(); i++) {
		BMessage* itemMsg = menuItems.ItemAt(i);
		BMessage* msg = new BMessage(*itemMsg);
		BMessage toSend;
		msg->FindMessage("_msg", &toSend);
		toSend.AddString("chat_id", fConversation->GetId());
		toSend.AddInt64("instance", looper->GetInstance());
		msg->ReplaceMessage("_msg", &toSend);

		BMenuItem* item = new BMenuItem(msg);
		if (item == NULL)
			continue;
		if (msg->GetBool("x_to_protocol", true) == true)
			item->SetTarget(looper);
		else
			item->SetTarget(this);
		menu->AddItem(item);
	}
	return menu;
}


void
MainWindow::_ToggleMenuItems()
{
	BMenuItem* chatMenuItem = fMenuBar->FindItem(B_TRANSLATE("Chat"));
	BMenuItem* rosterMenuItem = fMenuBar->FindItem(B_TRANSLATE("Roster"));
	BMenu* chatMenu = chatMenuItem->Submenu();
	BMenu* rosterMenu = rosterMenuItem->Submenu();

	if (chatMenu == NULL || rosterMenu == NULL)
		return;

	Server* server = Server::Get();
	bool enabled = (server != NULL && server->GetAccounts().CountItems() > 0);

	for (int i = 0; i < chatMenu->CountItems(); i++)
		chatMenu->ItemAt(i)->SetEnabled(enabled);

	for (int i = 0; i < rosterMenu->CountItems(); i++)
		rosterMenu->ItemAt(i)->SetEnabled(enabled);

	BMenuItem* windowMenuItem = fMenuBar->FindItem(B_TRANSLATE("Window"));
	BMenu* windowMenu = windowMenuItem->Submenu();
	enabled = (fListView->CountItems() > 0);

	for (int i = 0; i < windowMenu->CountItems(); i++)
		windowMenu->ItemAt(i)->SetEnabled(enabled);
}


ConversationItem*
MainWindow::_EnsureConversationItem(BMessage* msg)
{
	BString chat_id = msg->FindString("chat_id");
	Conversation* chat = Server::Get()->ConversationById(chat_id, msg->FindInt64("instance"));

	_EnsureConversationView(chat);

	// Add conversation item if necessary, return it
	ConversationItem* item;
	if (chat != NULL && (item = chat->GetListItem()) != NULL) {
		if (fListView->HasItem(item))
			fListView->InvalidateItem(fListView->IndexOf(item));
		else if (item != NULL) {
			fListView->AddConversation(chat);
			_ToggleMenuItems();
		}

		if (fListView->CountItems() == 1)
			fListView->Select(0);
		return item;
	}
	return NULL;
}


void
MainWindow::_EnsureConversationView(Conversation* chat)
{
	// Add conversation's view if necessary
	bool added = false;
	fChatList.ValueFor(chat, &added);

	if (added == false) {
		BLayoutItem* item = fChatLayout->AddView(chat->GetView());
		if (item != NULL)
			fChatList.AddItem(chat, item);
	}
}


void
MainWindow::_ApplyWeights()
{
	// Chat-list and chat-view splitter
	fSplitView->SetItemWeight(0, AppPreferences::Get()->MainWindowListWeight, true);
	fSplitView->SetItemWeight(1, AppPreferences::Get()->MainWindowChatWeight, true);

	// Chat-view's weights
	float horizChat, horizList, vertChat, vertSend;
	horizChat = AppPreferences::Get()->ChatViewHorizChatWeight;
	horizList = AppPreferences::Get()->ChatViewHorizListWeight;
	vertChat = AppPreferences::Get()->ChatViewVertChatWeight;
	vertSend = AppPreferences::Get()->ChatViewVertSendWeight;

	BLayoutItem* item = fChatLayout->VisibleItem();
	if (item != NULL)
		((ConversationView*)item->View())->SetWeights(horizChat, horizList, vertChat, vertSend);
}


void
MainWindow::_SaveWeights()
{
	// Chat-list and chat-view splitter
	AppPreferences::Get()->MainWindowListWeight = fSplitView->ItemWeight((int32)0);
	AppPreferences::Get()->MainWindowChatWeight = fSplitView->ItemWeight((int32)1);

	// ChatView's weights
	float horizChat, horizList, vertChat, vertSend;
	BLayoutItem* item = fChatLayout->VisibleItem();
	if (item == NULL)
		return;
	((ConversationView*)item->View())->GetWeights(&horizChat, &horizList, &vertChat, &vertSend);

	AppPreferences::Get()->ChatViewHorizChatWeight = horizChat;
	AppPreferences::Get()->ChatViewHorizListWeight = horizList;
	AppPreferences::Get()->ChatViewVertChatWeight = vertChat;
	AppPreferences::Get()->ChatViewVertSendWeight = vertSend;
}


bool
MainWindow::_PopulateWithAccounts(BMenu* menu, ProtocolSettings* settings)
{
	if (!settings)
		return false;
	BObjectList<BString> accounts = settings->Accounts();

	// Add accounts to menu
	for (int32 i = 0; i < accounts.CountItems(); i++) {
		BString* account = accounts.ItemAt(i);
		BMenu* accMenu = new BMenu(account->String());

		BString toggleLabel = B_TRANSLATE("Enable");
		bool isActive = false;
		int64 instance = Server::Get()->GetActiveAccounts().ValueFor(*account, &isActive);
		if (isActive == true)
			toggleLabel = B_TRANSLATE("Disable");

		BMessage* toggleMsg = new BMessage(APP_TOGGLE_ACCOUNT);
		toggleMsg->AddPointer("settings", settings);
		toggleMsg->AddString("account", *account);
		if (isActive == true)
			toggleMsg->AddInt64("instance", instance);

		BMessage* editMsg = new BMessage(APP_EDIT_ACCOUNT);
		editMsg->AddPointer("settings", settings);
		editMsg->AddString("account", *account);

		accMenu->AddItem(new BMenuItem(toggleLabel.String(), toggleMsg));

		accMenu->AddItem(
			new BMenuItem(B_TRANSLATE("Modify account" B_UTF8_ELLIPSIS),
				editMsg));

		menu->AddItem(accMenu);
	}
	return (accounts.CountItems() > 0);
}


void
MainWindow::_ReplaceMenu(const char* name, BMenu* newMenu)
{
	BMenuItem* old = fMenuBar->FindItem(name);
	if (old == NULL || newMenu == NULL)
		return;
	int32 index = fMenuBar->IndexOf(old);
	fMenuBar->RemoveItem(index);
	fMenuBar->AddItem(newMenu, index);
}
