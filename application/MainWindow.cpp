/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Application.h>
#include <Alert.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <ScrollView.h>
#include <TranslationUtils.h>

#include "AccountManager.h"
#include "CayaMessages.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "ConversationItem.h"
#include "ConversationListView.h"
#include "ConversationView.h"
#include "EditingFilter.h"
#include "JoinWindow.h"
#include "MainWindow.h"
#include "NotifyMessage.h"
#include "PreferencesDialog.h"
#include "ReplicantStatusView.h"
#include "RosterWindow.h"
#include "Server.h"
#include "StatusView.h"


const uint32 kLogin			= 'LOGI';


MainWindow::MainWindow()
	:
	BWindow(BRect(0, 0, 600, 400), "Caya", B_TITLED_WINDOW, 0),
	fWorkspaceChanged(false),
	fConversation(NULL),
	fRosterWindow(NULL)
{
	_InitInterface();

	// Filter messages using Server
	fServer = new Server();
	AddFilter(fServer);

	// Also through the editing filter (enter to send)
	AddCommonFilter(new EditingFilter(fSendView));
	fSendView->MakeFocus(true);

	CenterOnScreen();

	//TODO check for errors here
	ReplicantStatusView::InstallReplicant();
}


void
MainWindow::Start()
{
	// Login all accounts
	fServer->LoginAll();
}


bool
MainWindow::QuitRequested()
{
	int32 button_index = 0;
	if(!CayaPreferences::Item()->DisableQuitConfirm)
	{
		BAlert* alert = new BAlert("Closing", "Are you sure you want to quit?",
			"Yes", "No", NULL, B_WIDTH_AS_USUAL, B_OFFSET_SPACING,
			B_WARNING_ALERT);
		alert->SetShortcut(0, B_ESCAPE);
		button_index = alert->Go();
	}

	if(button_index == 0) {
		fServer->Quit();
		CayaPreferences::Get()->Save();
		ReplicantStatusView::RemoveReplicant();
		be_app->PostMessage(B_QUIT_REQUESTED);
		return true;
	} else {
		return false;
	}
}


void
MainWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case CAYA_SHOW_SETTINGS:
		{
			PreferencesDialog* dialog = new PreferencesDialog();
			dialog->Show();
			break;
		}

		case CAYA_NEW_CHAT:
		{
			fRosterWindow = new RosterWindow("Invite contact to chat"
			B_UTF8_ELLIPSIS, IM_CREATE_CHAT, new BMessenger(this), fServer);
			fRosterWindow->Show();
			break;
		}

		case CAYA_JOIN_CHAT:
		{
			JoinWindow* win = new JoinWindow(new BMessenger(this),
											 fServer->GetAccounts());
			win->Show();
			break;
		}

		case CAYA_MOVE_UP:
		{
			if (fConversation == NULL)
				return;

			int32 index = fListView->IndexOf(fConversation->GetListItem());
			if (index > 0)
				fListView->Select(index - 1);
			break;
		}

		case CAYA_MOVE_DOWN:
		{
			if (fConversation == NULL)
				return;

			int32 index = fListView->IndexOf(fConversation->GetListItem());
			int32 count = fListView->CountItems();
			if (index < (count - 1))
				fListView->Select(index + 1);
			break;
		}

		case CAYA_REPLICANT_STATUS_SET:
		{
			int32 status;
			message->FindInt32("status", &status);
			AccountManager* accountManager = AccountManager::Get();
			accountManager->SetStatus((CayaStatus)status);
			break;
		}

		case CAYA_REPLICANT_SHOW_WINDOW:
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

		case CAYA_CHAT:
		{
			message->AddString("body", fSendView->Text());
			fChatView->MessageReceived(message);
			fSendView->SetText("");

			break;
		}

		case IM_MESSAGE:
			ImMessage(message);
			break;
		case IM_ERROR:
			ImError(message);
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
			fStatusView->SetName(msg->FindString("name"));
			break;
		case IM_OWN_AVATAR_SET:
		{
			entry_ref ref;

			if (msg->FindRef("ref", &ref) == B_OK) {
				BBitmap* bitmap = BTranslationUtils::GetBitmap(&ref);
				fStatusView->SetAvatarIcon(bitmap);
			}
			break;
		}
		case IM_MESSAGE_RECEIVED:
		{
			_EnsureConversationItem(msg);
			break;
		}
		case IM_MESSAGE_SENT:
		case IM_CHAT_CREATED:
		{
			_EnsureConversationItem(msg);
			break;
		}
		case IM_AVATAR_SET:
		case IM_CONTACT_INFO:
		case IM_EXTENDED_CONTACT_INFO:
		case IM_STATUS_SET:
		{
			if (fRosterWindow != NULL)
				fRosterWindow->PostMessage(msg);
			break;
		}
	}
}


void
MainWindow::ImError(BMessage* msg)
{
	const char* error = NULL;
	const char* detail = msg->FindString("detail");

	if (msg->FindString("error", &error) != B_OK)
		return;

	// Format error message
	BString errMsg(error);
	if (detail)
		errMsg << "\n" << detail;

	BAlert* alert = new BAlert("Error", errMsg.String(), "OK", NULL, NULL,
		B_WIDTH_AS_USUAL, B_STOP_ALERT);
	alert->Go();
}


void
MainWindow::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_ACCOUNT_STATUS:
			fStatusView->SetStatus((CayaStatus)val);
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
	// Save current size of chat and textbox
	float weightChat = fRightView->ItemWeight(0);
	float weightSend = fRightView->ItemWeight(1);

	fRightView->RemoveChild(fRightView->FindView("chatView"));
	fRightView->RemoveChild(fRightView->FindView("fSendScroll"));

	if (chat != NULL) {
		fChatView = chat->GetView();
		fConversation = chat;
	}

	fRightView->AddChild(fChatView, 9);
	fRightView->AddChild(fSendScroll, 1);

	// Apply saved chat and textbox size to new views
	if (weightChat * weightSend != 0) {
		fRightView->SetItemWeight(0, weightChat, true);
		fRightView->SetItemWeight(1, weightSend, true);
	}
}


void
MainWindow::_InitInterface()
{
	// Left side of window, Roomlist + Status
	fListView = new ConversationListView("roomList");
	fStatusView = new StatusView("statusView");

	// Right-side of window, Chat + Textbox
	fRightView = new BSplitView(B_VERTICAL, 0);
	fChatView = new ConversationView();
	fSendView = new BTextView("fSendView");
	fSendScroll = new BScrollView("fSendScroll", fSendView,
		B_WILL_DRAW, false, true);
	fSendView->SetWordWrap(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(_CreateMenuBar())
		.AddGroup(B_HORIZONTAL)
			.SetInsets(5, 5, 0, 10)
			.AddSplit(B_HORIZONTAL, 0)
				.AddGroup(B_VERTICAL)
					.Add(fListView, 1)
					.Add(fStatusView)
				.End()
				.Add(fRightView, 5)
			.End()
		.End()
	.End();

	SetConversation(NULL);
}


BMenuBar*
MainWindow::_CreateMenuBar()
{
	BMenuBar* menuBar = new BMenuBar("MenuBar");

	// Program
	BMenu* programMenu = new BMenu("Program");
	programMenu->AddItem(new BMenuItem("About" B_UTF8_ELLIPSIS,
		new BMessage(B_ABOUT_REQUESTED)));
	programMenu->AddItem(new BMenuItem("Preferences" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_SHOW_SETTINGS), ',', B_COMMAND_KEY));
	programMenu->AddItem(new BSeparatorItem());
	programMenu->AddItem(new BMenuItem("Quit",
		new BMessage(B_QUIT_REQUESTED), 'Q', B_COMMAND_KEY));
	programMenu->SetTargetForItems(this);

	// Chat
	BMenu* chatMenu = new BMenu("Chat");
	BMenuItem* invite = new BMenuItem("Invite user" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_SEND_INVITE), 'I', B_COMMAND_KEY);
	invite->SetEnabled(false);
	BMenuItem* newRoom = new BMenuItem("New room" B_UTF8_ELLIPSIS,
		new BMessage(), 'N', B_COMMAND_KEY);
	newRoom->SetEnabled(false);

	chatMenu->AddItem(new BMenuItem("Join room" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_JOIN_CHAT), 'J', B_COMMAND_KEY));
	chatMenu->AddSeparatorItem();
	chatMenu->AddItem(new BMenuItem("New chat" B_UTF8_ELLIPSIS,
		new BMessage(CAYA_NEW_CHAT), 'M', B_COMMAND_KEY));
	chatMenu->AddItem(newRoom);
	chatMenu->AddSeparatorItem();
	chatMenu->AddItem(invite);
	chatMenu->SetTargetForItems(this);

	// Window
	BMenu* windowMenu = new BMenu("Window");
	windowMenu->AddItem(new BMenuItem("Up",
		new BMessage(CAYA_MOVE_UP), B_UP_ARROW, B_COMMAND_KEY));
	windowMenu->AddItem(new BMenuItem("Down",
		new BMessage(CAYA_MOVE_DOWN), B_DOWN_ARROW, B_COMMAND_KEY));
	windowMenu->SetTargetForItems(this);

	menuBar->AddItem(programMenu);
	menuBar->AddItem(chatMenu);
	menuBar->AddItem(windowMenu);
	return menuBar;
}


ConversationItem*
MainWindow::_EnsureConversationItem(BMessage* msg)
{
	ChatMap chats = fServer->Conversations();

	BString chat_id = msg->FindString("chat_id");
	Conversation* chat = fServer->ConversationById(chat_id);

	if (chat != NULL) {
		ConversationItem* item = chat->GetListItem();
		if (fListView->HasItem(item)) {
			_UpdateListItem(item);
		}
		else if (item != NULL) {
			fListView->AddItem(item);
		}

		if (fListView->CountItems() == 1)
			fListView->Select(0);
		return item;
	}

	return NULL;
}


void
MainWindow::_UpdateListItem(ConversationItem* item)
{
	if (fListView->HasItem(item) == true)
		fListView->InvalidateItem(fListView->IndexOf(item));
	else
		fListView->AddItem(item);
}


