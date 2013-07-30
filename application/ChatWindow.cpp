/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

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
#include <Notification.h>

#include "BitmapView.h"
#include "CayaMessages.h"
#include "CayaProtocolMessages.h"
#include "CayaPreferences.h"
#include "ChatWindow.h"
#include "ContactLinker.h"
#include "EditingFilter.h"
#include "CayaConstants.h"
#include "CayaRenderView.h"
#include "NotifyMessage.h"


ChatWindow::ChatWindow(ContactLinker* cl)
	:
	BWindow(BRect(200, 200, 500, 500),
		cl->GetName().String(), B_TITLED_WINDOW, 0),
		fContactLinker(cl)
{
	fMessageCount = 0;
	
	fReceiveView = new CayaRenderView("fReceiveView");
	fReceiveView->SetOtherNick(cl->GetName());
	BScrollView* scrollViewReceive = new BScrollView("scrollviewR",
		fReceiveView, B_WILL_DRAW, false, true);

	fSendView = new BTextView("fReceiveView");
	BScrollView* scrollViewSend = new BScrollView("scrollviewS", fSendView,
		B_WILL_DRAW, false, true);
	fSendView->SetWordWrap(true);
	AddCommonFilter(new EditingFilter(fSendView));
	fSendView->MakeFocus(true);


	fPersonalMessage = new BTextView("personalMessage", B_WILL_DRAW);
	fPersonalMessage->SetExplicitAlignment(
		BAlignment(B_ALIGN_LEFT, B_ALIGN_MIDDLE));

	fPersonalMessage->SetText(fContactLinker->GetNotifyPersonalStatus());
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
	fAvatar->SetBitmap(fContactLinker->AvatarBitmap());

	BBitmap* protocolBitmap = fContactLinker->ProtocolBitmap();
	BitmapView* protocolView = new BitmapView("protocolView");
	protocolView->SetBitmap(protocolBitmap);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 10)
		.AddGroup(B_HORIZONTAL)
			.Add(protocolView)
			.Add(fPersonalMessage)
			.Add(fAvatar)
		.End()
		.AddSplit(B_VERTICAL, 2.0f)
			.Add(scrollViewReceive, 3)
			.Add(scrollViewSend, 2)
		.End()
		.Add(fStatus, 4)
		.SetInsets(5, 5, 5, 5);

	MoveTo(BAlert::AlertPosition(Bounds().Width(), Bounds().Height() / 2));

	fSendView->MakeFocus(true);
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
	msg.AddString("id", fContactLinker->GetId());
	fContactLinker->Messenger().SendMessage(&msg);
	return false;
}


void
ChatWindow::UpdateAvatar()
{
	if (fContactLinker->AvatarBitmap() != NULL) {
		LockLooper();
		fAvatar->SetBitmap(fContactLinker->AvatarBitmap());
		UnlockLooper();
	}
}


void
ChatWindow::UpdatePersonalMessage()
{

	if (fContactLinker->GetNotifyPersonalStatus() != NULL) {
		LockLooper();
		fPersonalMessage->SetText(fContactLinker->GetNotifyPersonalStatus());
		UnlockLooper();
	}
}


void
ChatWindow::MessageReceived(BMessage* message)
{
	switch (message->what) {
		case CAYA_CHAT:
		{
			BString message = fSendView->Text();
			if (message == "")
				return;

			fReceiveView->AppendOwnMessage(message.String());

			BMessage msg(IM_MESSAGE);
			msg.AddInt32("im_what", IM_SEND_MESSAGE);
			msg.AddString("id", fContactLinker->GetId());
			msg.AddString("body", message);
			fContactLinker->Messenger().SendMessage(&msg);

			fSendView->SetText("");
			break;
		}
		case IM_MESSAGE:
			ImMessage(message);
			break;

		default:
			BWindow::MessageReceived(message);
			break;
	}
}


void
ChatWindow::ImMessage(BMessage* msg)
{
	int32 im_what = msg->FindInt32("im_what");

	switch (im_what) {
		case IM_MESSAGE_RECEIVED:
		{
			BString message = msg->FindString("body");
			fReceiveView->AppendOtherMessage(message.String());

			// Message received, clear status anyway
			fStatus->SetText("");
			
			if (IsActive()) break;
			
			fMessageCount++;
		
			// Mark unread window			
			if (CayaPreferences::Item()->MarkUnreadWindow) { 
				BString title = "[";
				title<<fMessageCount;
				title<<"] ";
				title<<fContactLinker->GetName();
				SetTitle(title);
			}
			
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
			notify_message << fContactLinker->GetName().String();

			BNotification notification(B_INFORMATION_NOTIFICATION);
			notification.SetGroup(BString("Caya"));
			notification.SetTitle(BString("New message"));
			notification.SetIcon(fAvatar->Bitmap());
			notification.SetContent(notify_message);
			notification.SetMessageID(fContactLinker->GetName());
			notification.Send();
			
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
ChatWindow::WindowActivated(bool active)
{
	SetTitle(fContactLinker->GetName());
	fMessageCount=0;
}

void
ChatWindow::ObserveString(int32 what, BString str)
{
	switch (what) {
		case STR_CONTACT_NAME:
			if (Lock()) {
				SetTitle(str);
				fReceiveView->SetOtherNick(str);
				Unlock();
			}
			break;
		case STR_PERSONAL_STATUS:
			break;
	}
}


void
ChatWindow::ObservePointer(int32 what, void* ptr)
{
	switch (what) {
		case PTR_AVATAR_BITMAP:
			break;
	}
}


void
ChatWindow::ObserveInteger(int32 what, int32 val)
{
	switch (what) {
		case INT_CONTACT_STATUS:
			if (Lock()) {
				AppendStatus((CayaStatus)val);
				Unlock();
			}
			break;
	}
}


void
ChatWindow::AppendStatus(CayaStatus status)
{
	BString message(fContactLinker->GetName());

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
