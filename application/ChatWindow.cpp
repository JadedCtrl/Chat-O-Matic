/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "ChatWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <GridLayout.h>
#include <GridLayoutBuilder.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <Layout.h>
#include <LayoutBuilder.h>
#include <ListView.h>
#include <Message.h>
#include <SpaceLayoutItem.h>
#include <ScrollView.h>
#include <String.h>
#include <StringList.h>
#include <Notification.h>

#include <libinterface/BitmapView.h>

#include "CayaMessages.h"
#include "CayaProtocolMessages.h"
#include "CayaPreferences.h"
#include "ConversationView.h"
#include "Conversation.h"
#include "Contact.h"
#include "EditingFilter.h"
#include "CayaConstants.h"
#include "CayaRenderView.h"
#include "NotifyMessage.h"

ChatWindow::ChatWindow()
	:
	BWindow(BRect(200, 200, 500, 500), "Chat", B_TITLED_WINDOW, 0)
{
	fChatView = new ConversationView();

	fSendView = new BTextView("fSendView");
	fSendScroll = new BScrollView("fSendScroll", fSendView,
		B_WILL_DRAW, false, true);
	fSendView->SetWordWrap(true);

	AddCommonFilter(new EditingFilter(fSendView));
	fSendView->MakeFocus(true);

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.Add(fChatView)
		.Add(fSendScroll)
	.End();
}


ChatWindow::ChatWindow(Conversation* cl)
	:
	ChatWindow()
{
	fChatView = new ConversationView(cl);
	SetConversation(cl);
}


void
ChatWindow::SetConversation(Conversation* chat)
{
	BView* current = FindView("chatView");
	RemoveChild(FindView("chatView"));
	RemoveChild(FindView("fSendScroll"));

	fChatView = chat->GetView();

	AddChild(fChatView);
	AddChild(fSendScroll);
}


void
ChatWindow::ShowWindow()
{
	if (IsHidden())
		Show();

	if (IsMinimized())
		Minimize(false);

	if (!IsActive())
		Activate(true);
}


bool
ChatWindow::QuitRequested()
{
	BMessage msg(CAYA_CLOSE_CHAT_WINDOW);
//	msg.AddString("chat_id", fConversation->GetId());
//	fConversation->Messenger().SendMessage(&msg);
	return false;
}


void
ChatWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case CAYA_CHAT:
			message->PrintToStream();
			message->AddString("body", fSendView->Text());
			fChatView->MessageReceived(message);
			fSendView->SetText("");
			break;

		case IM_MESSAGE:
			fChatView->ImMessage(message);
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
ChatWindow::ImMessage(BMessage* msg)
{
	fChatView->ImMessage(msg);
}


void
ChatWindow::ObserveString(int32 what, BString str)
{
	fChatView->ObserveString(what, str);
}


void
ChatWindow::ObservePointer(int32 what, void* ptr)
{
	fChatView->ObservePointer(what, ptr);
}


void
ChatWindow::ObserveInteger(int32 what, int32 val)
{
	fChatView->ObserveInteger(what, val);
}


void
ChatWindow::AvoidFocus(bool avoid)
{
	// This is needed to avoid the window focus when
	// a new message is received, since it could be a lot annoying
	// for the user
	if (avoid)
		SetFlags(B_AVOID_FOCUS);
	else
		SetFlags(Flags() &~ B_AVOID_FOCUS);
}


