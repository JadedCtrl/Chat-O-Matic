/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "ConversationView.h"

#include <Catalog.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <ScrollView.h>
#include <SplitView.h>
#include <StringList.h>
#include <StringView.h>

#include <libinterface/BitmapView.h>
#include <libinterface/EnterTextView.h>

#include "AppMessages.h"
#include "AppPreferences.h"
#include "ChatProtocolMessages.h"
#include "Conversation.h"
#include "NotifyMessage.h"
#include "ProtocolManager.h"
#include "RenderView.h"
#include "SendTextView.h"
#include "User.h"
#include "UserItem.h"
#include "UserListView.h"
#include "Utils.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConversationView"


ConversationView::ConversationView(Conversation* chat)
	:
	BGroupView("chatView", B_VERTICAL, B_USE_DEFAULT_SPACING),
	fMessageQueue(),
	fConversation(chat)
{
	_InitInterface();
	if (chat != NULL) {
		SetConversation(chat);
		fUserList->SetConversation(chat);
	}
	else
		_FakeChat();
}


bool
ConversationView::QuitRequested()
{
	BMessage msg(APP_CLOSE_CHAT_WINDOW);
	if (fConversation != NULL) {
		msg.AddString("chat_id", fConversation->GetId());
		fConversation->Messenger().SendMessage(&msg);
	}
	return false;
}


void
ConversationView::AttachedToWindow()
{
	while (fMessageQueue.IsEmpty() == false) {
		BMessage* msg = fMessageQueue.RemoveItemAt(0);
		ImMessage(msg);
	}
	if (fConversation != NULL) {
		if (fNameTextView->Text() != fConversation->GetName())
			fNameTextView->SetText(fConversation->GetName());
		if (fSubjectTextView->Text() != fConversation->GetSubject())
			fSubjectTextView->SetText(fConversation->GetSubject());
	}
	NotifyInteger(INT_WINDOW_FOCUSED, 0);
	fSendView->MakeFocus(true);
}


void
ConversationView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case APP_CHAT:
		{
			BString text = fSendView->Text();
			if (fConversation == NULL || text == "")
				return;
			int64 instance = fConversation->GetProtocolLooper()->GetInstance();

			BMessage msg(IM_MESSAGE);
			msg.AddInt32("im_what", IM_SEND_MESSAGE);
			msg.AddInt64("instance", instance);
			msg.AddString("chat_id", fConversation->GetId());
			msg.AddString("body", text);
			fConversation->ImMessage(&msg);

			fSendView->SetText("");
			break;
		}
		case IM_MESSAGE:
			ImMessage(message);
			break;

		default:
			BGroupView::MessageReceived(message);
			break;
	}
}


void
ConversationView::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");

	switch (im_what) {
		case IM_ROOM_LEFT:
		{
			delete fConversation;
			delete this;
			break;
		}
		case IM_MESSAGE_RECEIVED:
		{
			msg->AddInt64("when", (int64)time(NULL));
			_AppendOrEnqueueMessage(msg);
			fReceiveView->ScrollToBottom();
			break;
		}
		case IM_MESSAGE_SENT:
		case IM_LOGS_RECEIVED:
		{
			_AppendOrEnqueueMessage(msg);
			if (im_what == IM_MESSAGE_SENT)
				fReceiveView->ScrollToBottom();
			break;
		}
		case IM_ROOM_JOINED:
		{
			BMessage msg;
			msg.AddString("body", B_TRANSLATE("** You joined the room.\n"));
			msg.AddInt64("when", (int64)time(NULL));
			_AppendOrEnqueueMessage(&msg);
			fReceiveView->ScrollToBottom();
		}
		case IM_ROOM_CREATED:
		{
			BMessage msg;
			msg.AddString("body", B_TRANSLATE("** You created the room.\n"));
			msg.AddInt64("when", (int64)time(NULL));
			_AppendOrEnqueueMessage(&msg);
			fReceiveView->ScrollToBottom();
		}
		case IM_ROOM_PARTICIPANT_JOINED:
		{
			_UserMessage(B_TRANSLATE("%user% has joined the room.\n"),
						 B_TRANSLATE("%user% has joined the room (%body%).\n"),
						 msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_LEFT:
		{
			_UserMessage(B_TRANSLATE("%user% has left the room.\n"),
						 B_TRANSLATE("%user% has left the room (%body%).\n"),
						 msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_KICKED:
		{
			_UserMessage(B_TRANSLATE("%user% was kicked.\n"),
						 B_TRANSLATE("%user% was kicked (%body%).\n"),msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_BANNED:
		{
			_UserMessage(B_TRANSLATE("%user% has been banned.\n"),
						 B_TRANSLATE("%user% has been banned (%body%).\n"),
						 msg);
			break;
		}
		case IM_ROOM_ROLECHANGED:
		{
			BString user_id = msg->FindString("user_id");

			if (user_id == fConversation->GetOwnContact()->GetId()) {
				Role* role = fConversation->GetRole(user_id);
				if (role == NULL)
					break;
				int32 perms = role->fPerms;
				fNameTextView->MakeEditable(perms & PERM_ROOM_NAME);
				fSubjectTextView->MakeEditable(perms & PERM_ROOM_SUBJECT);
			}
			break;
		}
		case IM_SET_ROOM_NAME:
		case IM_SET_ROOM_SUBJECT:
		{
			if (fConversation == NULL)
				return;
			fConversation->GetProtocolLooper()->MessageReceived(msg);

			// Reset to current values; if the change went through, it'll
			// come back.
			fNameTextView->SetText(fConversation->GetName());
			fSubjectTextView->SetText(fConversation->GetSubject());
			break;
		}
		case IM_PROTOCOL_READY:
		{
			fReceiveView->SetText("");
			_FakeChatNoRooms();
		}
		default:
			break;
	}
}


Conversation*
ConversationView::GetConversation()
{
	return fConversation;
}


void
ConversationView::SetConversation(Conversation* chat)
{
	if (chat == NULL)
		return;
	fConversation =  chat;

	BMessage name(IM_MESSAGE);
	name.AddInt32("im_what", IM_SET_ROOM_NAME);
	name.AddString("chat_id", chat->GetId());
	fNameTextView->SetText(chat->GetName());
	fNameTextView->SetMessage(name, "chat_name");
	fNameTextView->SetTarget(this);

	BMessage subject(IM_MESSAGE);
	subject.AddInt32("im_what", IM_SET_ROOM_SUBJECT);
	subject.AddString("chat_id", chat->GetId());
	fSubjectTextView->SetText(chat->GetSubject());
	fSubjectTextView->SetMessage(subject, "subject");
	fSubjectTextView->SetTarget(this);

	fProtocolView->SetBitmap(chat->ProtocolBitmap());
}


void
ConversationView::UpdateIcon()
{
	if (fConversation != NULL && fConversation->IconBitmap() != NULL)
		fIcon->SetBitmap(fConversation->IconBitmap());
}


void
ConversationView::UpdateUserList(UserMap users)
{
	fUserList->MakeEmpty();
	for (int i = 0; i < users.CountItems(); i++) {
		User* user = users.ValueAt(i);
		if (fUserList->HasItem(user->GetListItem()) == false) {
			fUserList->AddItem(user->GetListItem());
			fUserList->Sort();
		}
	}
}


void
ConversationView::InvalidateUserList()
{
	for (int i = 0; i < fUserList->CountItems(); i++)
		fUserList->InvalidateItem(i);
}


void
ConversationView::ObserveString(int32 what, BString str)
{
	switch (what)
	{
		case STR_ROOM_NAME:
		{
			fNameTextView->SetText(str);
			break;
		}
		case STR_ROOM_SUBJECT:
		{
			fSubjectTextView->SetText(str);
			break;
		}
	}
}


void
ConversationView::GetWeights(float* horizChat, float* horizList,
	float* vertChat, float* vertSend)
{
	*horizChat = fHorizSplit->ItemWeight(0);
	*horizList = fHorizSplit->ItemWeight(1);
	*vertChat = fVertSplit->ItemWeight(0);
	*vertSend = fVertSplit->ItemWeight(1);
}


void
ConversationView::SetWeights(float horizChat, float horizList, float vertChat,
	float vertSend)
{
	fHorizSplit->SetItemWeight(0, horizChat, true);
	fHorizSplit->SetItemWeight(1, horizList, true);
	fVertSplit->SetItemWeight(0, vertChat, true);
	fVertSplit->SetItemWeight(1, vertSend, true);
}


void
ConversationView::_InitInterface()
{
	fReceiveView = new RenderView("receiveView");
	BScrollView* scrollViewReceive = new BScrollView("receiveScrollView",
		fReceiveView, B_WILL_DRAW, false, true);

	fSendView = new SendTextView("sendView", this);

	fNameTextView = new EnterTextView("roomName", be_bold_font, NULL, B_WILL_DRAW);
	fNameTextView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fNameTextView->SetStylable(true);
	fNameTextView->MakeEditable(false);
	fNameTextView->MakeResizable(true);

	fSubjectTextView = new EnterTextView("roomSubject");
	fSubjectTextView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fSubjectTextView->MakeEditable(false);
	fSubjectTextView->MakeResizable(true);

	fIcon = new BitmapView("ContactIcon");
	fIcon->SetExplicitMinSize(BSize(50, 50));
	fIcon->SetExplicitPreferredSize(BSize(50, 50));
	fIcon->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));
	fIcon->SetSquare(true);

	fProtocolView = new BitmapView("protocolView");

	fUserList = new UserListView("userList");
	BScrollView* scrollViewUsers = new BScrollView("userScrollView",
		fUserList, B_WILL_DRAW, false, true);

	fHorizSplit = new BSplitView(B_HORIZONTAL, 0);
	fVertSplit = new BSplitView(B_VERTICAL, 0);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.Add(fIcon)
			.AddGroup(B_VERTICAL)
				.Add(fNameTextView)
				.Add(fSubjectTextView)
			.End()
			.Add(fProtocolView)
		.End()
		.AddSplit(fHorizSplit, 0.0)
			.AddGroup(B_VERTICAL)
				.AddSplit(fVertSplit, 8.0)
					.Add(scrollViewReceive, 20)
					.Add(fSendView, 1)
				.End()
			.End()
			.Add(scrollViewUsers, 1)
		.End()
	.End();
}


bool
ConversationView::_AppendOrEnqueueMessage(BMessage* msg)
{
	// If not attached to the chat window, then re-handle this message
	// later [AttachedToWindow()], since you can't edit an unattached 
	// RenderView.
	if (Window() == NULL) {
		fMessageQueue.AddItem(new BMessage(*msg));
		return false;
	}

	// Alright, we're good to append!
	_AppendMessage(msg);
	return true;
}


void
ConversationView::_AppendMessage(BMessage* msg)
{
	BStringList users, bodies;
	if (msg->FindStrings("body", &bodies) != B_OK)
		return;
	msg->FindStrings("user_id", &users);

	for (int i = bodies.CountStrings(); i >= 0; i--) {
		User* sender = NULL;
		if (fConversation != NULL)
			sender = fConversation->UserById(users.StringAt(i));
		BString sender_name = users.StringAt(i);
		BString body = bodies.StringAt(i);
		rgb_color userColor = ui_color(B_PANEL_TEXT_COLOR);
		int64 timeInt;

		if (msg->FindInt64("when", i, &timeInt) != B_OK)
			timeInt = (int64)time(NULL);

		if (sender != NULL) {
			sender_name = sender->GetName();
			userColor = sender->fItemColor;
		}

		if (sender_name.IsEmpty() == true) {
			fReceiveView->AppendGeneric(body.String());
			continue;
		}

		fReceiveView->AppendMessage(sender_name.String(), body.String(),
									userColor, (time_t)timeInt);
	}
}


void
ConversationView::_UserMessage(const char* format, const char* bodyFormat,
							   BMessage* msg)
{
	BString user_id;
	BString user_name = msg->FindString("user_name");
	BString body = msg->FindString("body");

	if (msg->FindString("user_id", &user_id) != B_OK)
		return;
	if (user_name.IsEmpty() == true)
		user_name = user_id;

	BString newBody("** ");
	if (body.IsEmpty() == true)
		newBody << format;
	else {
		newBody << bodyFormat;
		newBody.ReplaceAll("%body%", body.String());
	}
	newBody.ReplaceAll("%user%", user_name.String());

	BMessage newMsg;
	newMsg.AddString("body", newBody);
	newMsg.AddInt64("when", (int64)time(NULL));
	_AppendOrEnqueueMessage(&newMsg);
	fReceiveView->ScrollToBottom();
}


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConversationView ― Startup messages"


void
ConversationView::_FakeChat()
{
	if (ProtocolManager::Get()->CountProtocolInstances() <= 0)
		_FakeChatNoAccounts();
	else
		_FakeChatNoRooms();
}


void
ConversationView::_FakeChatNoRooms()
{
	fNameTextView->SetText(B_TRANSLATE("Cardie"));
	fSubjectTextView->SetText(B_TRANSLATE("No current rooms or chats."));

	BMessage welcome(IM_MESSAGE);
	welcome.AddInt32("im_what", IM_MESSAGE_RECEIVED);

	welcome.AddString("user_id", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", B_TRANSLATE("You know, only if you want. I'm not trying to be pushy."));

	welcome.AddString("user_id", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", B_TRANSLATE("You can join or create one through the Chat menu.. :-)"));

	welcome.AddString("user_id", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", B_TRANSLATE("You aren't in any rooms or chats right now."));
	_AppendOrEnqueueMessage(&welcome);
}

void
ConversationView::_FakeChatNoAccounts()
{
	fNameTextView->SetText(B_TRANSLATE("Cardie setup"));
	fSubjectTextView->SetText(B_TRANSLATE("No accounts configured, no joy."));

	BMessage welcome(IM_MESSAGE);
	welcome.AddInt32("im_what", IM_MESSAGE_RECEIVED);

	welcome.AddString("user_id", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", B_TRANSLATE("Afterward, you can join a room or start a chat through the Chat menu. :-)"));

	welcome.AddString("user_id", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", B_TRANSLATE("Add an account through the [Program→Preferences] menu to get started."));

	welcome.AddString("user_id", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", B_TRANSLATE("It looks like you don't have any accounts set up."));
	_AppendOrEnqueueMessage(&welcome);
}
