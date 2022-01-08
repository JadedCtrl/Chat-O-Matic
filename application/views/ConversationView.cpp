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


void
ConversationView::AttachedToWindow()
{
	while (fMessageQueue.IsEmpty() == false) {
		BMessage* msg = fMessageQueue.RemoveItemAt(0);
		MessageReceived(msg);
	}
	if (fConversation != NULL) {
		if (fNameTextView->Text() != fConversation->GetName())
			fNameTextView->SetText(fConversation->GetName());
		if (fSubjectTextView->Text() != fConversation->GetSubject())
			fSubjectTextView->SetText(fConversation->GetSubject());
	}
	NotifyInteger(INT_WINDOW_FOCUSED, 0);
	fSendView->MakeFocus(true);
	fSendView->Invalidate();
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
			fSendView->ScrollToOffset(0);
			break;
		}
		case kClearText:
			_AppendOrEnqueueMessage(message);
			break;
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
ConversationView::UpdateUserList(UserMap users)
{
	fUserList->MakeEmpty();
	for (int i = 0; i < users.CountItems(); i++) {
		User* user = users.ValueAt(i);
		if (fUserList->HasUser(user) == false) {
			fUserList->AddUser(user);
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

			BString body = B_TRANSLATE("** The subject is now: %subject%");
			body.ReplaceAll("%subject%", str);

			BMessage topic(IM_MESSAGE);
			topic.AddString("body", body);
			_AppendOrEnqueueMessage(&topic);
			break;
		}
	}
}


void
ConversationView::ObservePointer(int32 what, void* ptr)
{
	switch (what)
	{
		case PTR_ROOM_BITMAP:
		{
			if (ptr != NULL)		
				fIcon->SetBitmap((BBitmap*)ptr);
			break;
		}
	}
}


void
ConversationView::GetWeights(float* horizChat, float* horizList,
	float* vertChat, float* vertSend)
{
	*horizChat = fHorizSplit->ItemWeight((int32)0);
	*horizList = fHorizSplit->ItemWeight((int32)1);
	*vertChat = fVertSplit->ItemWeight((int32)0);
	*vertSend = fVertSplit->ItemWeight((int32)1);
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
		fReceiveView, B_WILL_DRAW, false, true, B_NO_BORDER);

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
	// Fill the message with user information not provided by protocol
	BString user_id = msg->FindString("user_id");
	if (msg->FindString("user_id", &user_id) == B_OK) {
		User* user = NULL;
		if (fConversation != NULL)
			user = fConversation->UserById(user_id);
		if (user != NULL) {
			if (msg->HasString("user_name") == false)
				if (user->GetName().IsEmpty() == false)
					msg->AddString("user_name", user->GetName());
			msg->AddColor("user_color", user->fItemColor);
		}
		if (msg->HasString("user_name") == false)
			msg->AddString("user_name", user_id);
	}

	// Fill the message with receive time if not provided
	if (msg->HasInt64("when") == false)
		msg->AddInt64("when", (int64)time(NULL));

	// If not attached to the chat window, then re-handle this message
	// later [AttachedToWindow()], since you can't edit an unattached 
	// RenderView.
	if (Window() == NULL) {
		// If contains multiple chat messages (e.g., IM_LOGS_RECEIVED), add all
		int32 i = -1;
		BMessage text;
		while (msg->FindMessage("message", i + 1, &text) == B_OK) {
			fMessageQueue.AddItem(new BMessage(text));
			i++;
		}
		// Else, add the lonely, lonely, single-messaged one
		if (i == -1)
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
	// If ordered to clear buffer… well, I guess we can't refuse
	if (msg->what == kClearText) {
		fReceiveView->SetText("");
		return;
	}

	// Otherwise, it's message time!
	int64 timeInt = msg->GetInt64("when", time(NULL));
	BString user_name = msg->FindString("user_name");
	rgb_color userColor = msg->GetColor("user_color", ui_color(B_PANEL_TEXT_COLOR));
	BString body;

	if (msg->FindString("body", &body) != B_OK)
		return;

	if (user_name.IsEmpty() == true) {
		fReceiveView->AppendGeneric(body, timeInt);
		return;
	}

	if (body.StartsWith("/me ")) {
		BString meMsg = "** ";
		meMsg << user_name.String() << " ";
		meMsg << body.RemoveFirst("/me ");
		fReceiveView->AppendGeneric(meMsg.String(), timeInt);
		return;
	}

	fReceiveView->AppendTimestamp(timeInt);
	fReceiveView->AppendUserstamp(user_name, userColor);

	// And here we append the body…
	uint16 face = 0;
	UInt16IntMap face_indices;
	rgb_color color = ui_color(B_PANEL_TEXT_COLOR);
	int32 colorIndice = -1;
	int32 next = body.CountChars();

	BFont font;
	for (int i = 0; i < body.CountChars(); i++) {
		_DisableEndingFaces(msg, i, &face, &face_indices);
		_EnableStartingFaces(msg, i, &face, &face_indices, &next);
		_EnableStartingColor(msg, i, &color, &colorIndice, &next);

		if (face == B_REGULAR_FACE) {
			font = BFont();
			face = 0;
		}
		else if (face > 0) {
			font.SetFace(face);
		}

		if (colorIndice > 0 && colorIndice <= i) {
			color = ui_color(B_PANEL_TEXT_COLOR);
			colorIndice == -1;
		}

		// If formatting doesn't change for a while, send text in bulk
		if ((next - i) > 1) {
			BString append;
			body.CopyCharsInto(append, i, next - i);
			fReceiveView->Append(append, color, &font);
			i = next - 1;
			next = body.CountChars();
		}
		// Otherwise, send only current character
		else {
			int32 bytes;
			const char* curChar = body.CharAt(i, &bytes);
			char append[bytes];

			for (int i = 0; i < bytes; i++)
				append[i] = curChar[i];
			append[bytes] = '\0';
			fReceiveView->Append(append, color, &font);
		}

		if (next < i)
			next = body.CountChars() - 1;

	}
	fReceiveView->Append("\n");
}


void
ConversationView::_EnableStartingFaces(BMessage* msg, int32 index, uint16* face,
	UInt16IntMap* indices, int32* next)
{
	int32 face_start;
	int32 face_length;
	uint16 newFace;

	int32 i = 0;
	while (msg->FindInt32("face_start", i, &face_start) == B_OK) {
		// Change 'next' value for new fonts
		if (face_start > index && face_start < *next)
			*next = face_start;

		// Set face normally
		if (face_start == index) {
			if (msg->FindInt32("face_length", i, &face_length) != B_OK)
				continue;
			if (msg->FindUInt16("face", i, &newFace) != B_OK)
				continue;

			*face |= newFace;
			indices->AddItem(newFace, index + face_length);
		}
		i++;
	}

	// Change 'next' for ending old fonts
	for (int i = 0; i < indices->CountItems(); i++) {
		int faceEnd = indices->ValueAt(i);
		if (faceEnd > 0 && faceEnd > index && faceEnd < *next)
			*next = faceEnd;
	}
}


void
ConversationView::_DisableEndingFaces(BMessage* msg, int32 index, uint16* face,
	UInt16IntMap* indices)
{
	for (int32 i = 0; i < indices->CountItems(); i++) {
		uint16 key = indices->KeyAt(i);
		int32 value = indices->ValueAt(i);

		if (value <= index) {
			indices->RemoveItemAt(i);
			*face = *face & ~key;
			if (indices->CountItems() == 0)
				*face = B_REGULAR_FACE;
		}
	}
}


void
ConversationView::_EnableStartingColor(BMessage* msg, int32 index,
	rgb_color* color, int32* indice, int32* next)
{
	rgb_color newColor;
	int32 color_start, color_length, i = 0;
	while (msg->FindInt32("color_start", i, &color_start) == B_OK) {
		if (color_start > index && color_start < *next)
			*next = color_start - 1;

		if (color_start == index
				&& msg->FindInt32("color_length", i, &color_length) == B_OK
				&& msg->FindColor("color", i, &newColor) == B_OK)
		{
			*indice = color_length + index;
			*color = newColor;
			if (*indice > index && (*indice < *next || *next < index))
				*next = *indice;
			break;
		}
		i++;
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
	fReceiveView->ScrollToBottom();
}


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConversationView ― Startup messages"


void
ConversationView::_FakeChat()
{
	BString name(B_TRANSLATE("%app% setup"));
	name.ReplaceAll("%app%", APP_NAME);
	fNameTextView->SetText(name);
	fSubjectTextView->SetText(B_TRANSLATE("No accounts enabled, no joy."));

	BMessage obsv(IM_MESSAGE);
	obsv.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	obsv.AddString("user_name", B_TRANSLATE("Master Foo"));
	obsv.AddString("body", B_TRANSLATE("It looks like you don't have any accounts enabled."));
	_AppendOrEnqueueMessage(&obsv);

	BString body = B_TRANSLATE("Manage accounts through the %menu% menu to get started.");
	BString menu = B_TRANSLATE("Accounts");
	int32 boldStart = body.FindFirst("%");
	int32 boldLength = menu.CountChars();
	body.ReplaceAll("%menu%", menu);

	BMessage add(IM_MESSAGE);
	add.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	add.AddString("user_name", B_TRANSLATE("Master Foo"));
	add.AddString("body", body);
	add.AddInt32("face_start", boldStart);
	add.AddInt32("face_length", boldLength);
	add.AddUInt16("face", B_BOLD_FACE);
	_AppendOrEnqueueMessage(&add);

	body = B_TRANSLATE("Afterward, you can join a room or start a chat through the %menu% menu. :-)");
	menu = B_TRANSLATE("Chat");
	boldStart = body.FindFirst("%");
	boldLength = menu.CountChars();
	body.ReplaceAll("%menu%", menu);

	BMessage welcome(IM_MESSAGE);
	welcome.AddInt32("im_what", IM_MESSAGE_RECEIVED);
	welcome.AddString("user_name", B_TRANSLATE("Master Foo"));
	welcome.AddString("body", body);
	welcome.AddInt32("face_start", boldStart);
	welcome.AddInt32("face_length", boldLength);
	welcome.AddUInt16("face", B_BOLD_FACE);
	_AppendOrEnqueueMessage(&welcome);
}
