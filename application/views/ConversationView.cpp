/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "ConversationView.h"

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

#include "CayaConstants.h"
#include "CayaMessages.h"
#include "CayaPreferences.h"
#include "CayaProtocolMessages.h"
#include "CayaRenderView.h"
#include "Contact.h"
#include "Conversation.h"
#include "EditingFilter.h"
#include "NotifyMessage.h"


ConversationView::ConversationView(Conversation* chat)
	:
	BGroupView("chatView", B_VERTICAL, B_USE_DEFAULT_SPACING),
	fConversation(chat),
	fContact(chat->Users().ValueAt(0))
{
	fMessageCount = 0;
	
	fReceiveView = new CayaRenderView("fReceiveView");
	BScrollView* scrollViewReceive = new BScrollView("scrollviewR",
		fReceiveView, B_WILL_DRAW, false, true);


	fPersonalMessage = new BTextView("personalMessage", B_WILL_DRAW);
	fPersonalMessage->SetExplicitAlignment(
		BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));

	fPersonalMessage->SetText("");
	fPersonalMessage->SetExplicitMaxSize(BSize(400, 200));
	fPersonalMessage->MakeEditable(false);
	fPersonalMessage->SetViewColor(ui_color(B_PANEL_BACKGROUND_COLOR));

	fStatus = new BStringView("status", "");
	fStatus->SetExplicitAlignment(BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));

	fAvatar = new BitmapView("ContactIcon");
	fAvatar->SetExplicitMaxSize(BSize(50, 50));
	fAvatar->SetExplicitMinSize(BSize(50, 50));
	fAvatar->SetExplicitPreferredSize(BSize(50, 50));
	fAvatar->SetExplicitAlignment(BAlignment(B_ALIGN_RIGHT, B_ALIGN_MIDDLE));

	fProtocolView = new BitmapView("protocolView");

	BLayoutBuilder::Group<>(this, B_VERTICAL)
		.AddGroup(B_HORIZONTAL)
			.Add(fProtocolView)
			.Add(fPersonalMessage)
			.Add(fAvatar)
		.End()
		.Add(scrollViewReceive, 3)
		.Add(fStatus, 4)
		.SetInsets(5, 5, 5, 5);
}


bool
ConversationView::QuitRequested()
{
	BMessage msg(CAYA_CLOSE_CHAT_WINDOW);
	msg.AddString("chat_id", fConversation->GetId());
	fConversation->Messenger().SendMessage(&msg);
	return false;
}


Conversation*
ConversationView::GetConversation()
{
	return fConversation;
}


void
ConversationView::UpdateAvatar()
{
	if (fContact->AvatarBitmap() != NULL)
		fAvatar->SetBitmap(fContact->AvatarBitmap());
}


void
ConversationView::UpdatePersonalMessage()
{
	if (fContact->GetNotifyPersonalStatus() != NULL)
		fPersonalMessage->SetText(fContact->GetNotifyPersonalStatus());
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

			fReceiveView->AppendOwnMessage(text.String());

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
		case IM_MESSAGE_RECEIVED:
		{
			BString message = msg->FindString("body");
			BString id = msg->FindString("user_id");
			User* sender = fConversation->UserById(id);
			BString uname = sender->GetName();

			fReceiveView->AppendOtherMessage(uname.String(), message.String());

			// Message received, clear status anyway
			fStatus->SetText("");
			
			fMessageCount++;
			
			// Check if the user want the notification
			if (!CayaPreferences::Item()->NotifyNewMessage)
				break;

			BString notify_message;
			notify_message << "You've got ";
			notify_message << fMessageCount;
			if (fMessageCount==1) {
				notify_message << " new message from ";
			} else {
				notify_message << " new messages from ";
			};
			notify_message << uname;

			BNotification notification(B_INFORMATION_NOTIFICATION);
			notification.SetGroup(BString("Caya"));
			notification.SetTitle(BString("New message"));
			notification.SetIcon(sender->AvatarBitmap());
			notification.SetContent(notify_message);
			notification.SetMessageID(uname);
			notification.Send();
			
			break;
		}
		case IM_LOGS_RECEIVED:
		{
			BStringList logs;
			if (msg->FindStrings("log", &logs) != B_OK)
				return;

			for (int i = logs.CountStrings(); i >= 0; i--)
				fReceiveView->AppendGenericMessage(logs.StringAt(i).String());

			break;
		}
		case IM_CONTACT_STARTED_TYPING:
			fStatus->SetText("Contact is typing...");
			break;
		case IM_CONTACT_STOPPED_TYPING:
			fStatus->SetText("");
			break;
		case IM_CONTACT_GONE:
			fStatus->SetText("Contact closed the chat window!");
			snooze(10000);
			fStatus->SetText("");
			break;
		default:
			break;
	}
}


void
ConversationView::ObserveString(int32 what, BString str)
{
//	switch (what) {
//		case STR_CONTACT_NAME:
//			if (Lock()) {
//				SetTitle(str);
//				Unlock();
//			}
//			break;
//		case STR_PERSONAL_STATUS:
//			break;
//	}
}


void
ConversationView::ObservePointer(int32 what, void* ptr)
{
//	switch (what) {
//		case PTR_AVATAR_BITMAP:
//			break;
//	}
}


void
ConversationView::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_CONTACT_STATUS:
			AppendStatus((CayaStatus)val);
			break;
	}
}


void
ConversationView::AppendStatus(CayaStatus status)
{
	BString message(fContact->GetName());

	switch (status) {
		case CAYA_ONLINE:
			message << " is available";
			break;
		case CAYA_AWAY:
			message << " is away";
			break;
		case CAYA_DO_NOT_DISTURB:
			message << " is busy, please do not disturb!";
			break;
		case CAYA_CUSTOM_STATUS:
			message << " has set a custom status.";
			break;
		case CAYA_INVISIBLE:
			message << " is invisible.";
			break;
		case CAYA_OFFLINE:
			message << " is offline";
			break;
		default:
			break;
	}

	fReceiveView->Append(message.String(), COL_TEXT, COL_TEXT, R_TEXT);
 	fReceiveView->Append("\n", COL_TEXT, COL_TEXT, R_TEXT);
	fReceiveView->ScrollToSelection();
}


