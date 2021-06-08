/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "ConversationView.h"

#include <LayoutBuilder.h>
#include <ListView.h>
#include <Notification.h>
#include <ScrollView.h>
#include <StringList.h>
#include <StringView.h>

#include <libinterface/BitmapView.h>

#include "CayaMessages.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "CayaRenderView.h"
#include "CayaUtils.h"
#include "Conversation.h"
#include "NotifyMessage.h"
#include "User.h"
#include "UserItem.h"
#include "UserListView.h"


ConversationView::ConversationView()
	:
	BGroupView("chatView", B_VERTICAL, B_USE_DEFAULT_SPACING),
	fMessageQueue()
{
	fMessageCount = 0;
	_InitInterface();
}


ConversationView::ConversationView(Conversation* chat)
	: ConversationView()
{
	SetConversation(chat);
	fUserList->SetConversation(chat);
}


bool
ConversationView::QuitRequested()
{
	BMessage msg(CAYA_CLOSE_CHAT_WINDOW);
	msg.AddString("chat_id", fConversation->GetId());
	fConversation->Messenger().SendMessage(&msg);
	return false;
}


void
ConversationView::AttachedToWindow()
{
	while (fMessageQueue.IsEmpty() == false) {
		BMessage* msg = fMessageQueue.RemoveItemAt(0);
		ImMessage(msg);
	}
}


void
ConversationView::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case CAYA_CHAT:
		{
			BString text = message->FindString("body");
			if (text == "")
				return;

			BMessage msg(IM_MESSAGE);
			msg.AddInt32("im_what", IM_SEND_MESSAGE);
			msg.AddString("chat_id", fConversation->GetId());
			msg.AddString("body", text);
			fConversation->ImMessage(&msg);
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
			BString message = msg->FindString("body");
			BString id = msg->FindString("user_id");
			User* sender = fConversation->UserById(id);
			BString uname = sender->GetName();

			// Send a notification, if it's appropriate
			if ((Window() == NULL || Window()->IsActive() == false)
				&& (!CayaPreferences::Item()->NotifyNewMessage))
			{
				fMessageCount++;
				BString notify_message;
				notify_message << "You've got ";
				notify_message << fMessageCount;
				if (fMessageCount==1)
					notify_message << " new message from ";
				else
					notify_message << " new messages from ";
				notify_message << uname;

				BNotification notification(B_INFORMATION_NOTIFICATION);
				notification.SetGroup(BString("Caya"));
				notification.SetTitle(BString("New message"));
				notification.SetIcon(sender->AvatarBitmap());
				notification.SetContent(notify_message);
				notification.SetMessageID(uname);
				notification.Send();
				// Check if the user want the notification
				if (!CayaPreferences::Item()->NotifyNewMessage)
					break;
			}

			_AppendOrEnqueueMessage(msg);
			break;
		}
		case IM_MESSAGE_SENT:
		case IM_LOGS_RECEIVED:
		{
			_AppendOrEnqueueMessage(msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_JOINED:
		{
			_UserMessage("%user% has joined the room.\n",
						 "%user% has joined the room (%body%).\n", msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_LEFT:
		{
			_UserMessage("%user% has left the room.\n",
						 "%user% has left the room (%body%).\n", msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_KICKED:
		{
			_UserMessage("%user% was kicked.\n",
						 "%user% was kicked (%body%).\n", msg);
			break;
		}
		case IM_ROOM_PARTICIPANT_BANNED:
		{
			_UserMessage("%user% has been banned.\n",
						 "%user% has been banned (%body%).\n", msg);
			break;
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
	fConversation =  chat;
	fNameTextView->SetText(chat->GetName());
	fProtocolView->SetBitmap(chat->ProtocolBitmap());
}


void
ConversationView::UpdateIcon()
{
	if (fConversation->IconBitmap() != NULL)
		fIcon->SetBitmap(fConversation->IconBitmap());
}


void
ConversationView::UpdateUserList(UserMap users)
{
	fUserList->MakeEmpty();
	for (int i = 0; i < users.CountItems(); i++) {
		User* user = users.ValueAt(i);
		if (fUserList->HasItem(user->GetListItem()) == false)
			fUserList->AddItem(user->GetListItem());
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
		case STR_ROOM_SUBJECT:
		{
			fSubjectTextView->SetText(str);
			break;
		}
	}
}


void
ConversationView::_InitInterface()
{
	fReceiveView = new CayaRenderView("fReceiveView");
	BScrollView* scrollViewReceive = new BScrollView("receiveScrollView",
		fReceiveView, B_WILL_DRAW, false, true);

	fNameTextView = new BTextView("roomName", B_WILL_DRAW);
	fNameTextView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fNameTextView->MakeEditable(false);

	fSubjectTextView = new BTextView("roomSubject", B_WILL_DRAW);
	fSubjectTextView->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fSubjectTextView->MakeEditable(false);

	fIcon = new BitmapView("ContactIcon");
	fIcon->SetExplicitMinSize(BSize(50, 50));
	fIcon->SetExplicitPreferredSize(BSize(50, 50));
	fIcon->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));

	fProtocolView = new BitmapView("protocolView");

	fUserList = new UserListView("userList");
	BScrollView* scrollViewUsers = new BScrollView("userScrollView",
		fUserList, B_WILL_DRAW, false, true);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.Add(fIcon)
			.AddGroup(B_VERTICAL)
				.Add(fNameTextView)
				.Add(fSubjectTextView)
			.End()
			.Add(fProtocolView)
		.End()
		.AddSplit(B_HORIZONTAL, 0)
			.Add(scrollViewReceive, 5)
			.Add(scrollViewUsers, 1)
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
		User* sender = fConversation->UserById(users.StringAt(i));
		BString sender_name = users.StringAt(i);
		BString body = bodies.StringAt(i);
		rgb_color userColor = ui_color(B_PANEL_TEXT_COLOR);

		if (sender != NULL) {
			sender_name = sender->GetName();
			userColor = sender->fItemColor;
		}

		if (sender_name.IsEmpty() == true) {
			fReceiveView->AppendGenericMessage(body.String());
			continue;
		}

		fReceiveView->AppendMessage(sender_name.String(), body.String(), userColor);
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
	_AppendOrEnqueueMessage(&newMsg);
}


