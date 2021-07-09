/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SendTextView.h"

#include <StringList.h>
#include <Window.h>

#include "AppMessages.h"


SendTextView::SendTextView(const char* name, ConversationView* convView)
	:
	BTextView(name),
	fConversationView(convView),
	fCurrentIndex(0)
{
}


void
SendTextView::KeyDown(const char* bytes, int32 numBytes)
{
	int32 modifiers = Window()->CurrentMessage()->GetInt32("modifiers", 0);

	if (bytes[0] == B_TAB) {
		_AutoComplete();
		return;
	}
	// Reset auto-complete state if user typed/sent something
	fCurrentIndex = 0;
	fCurrentWord.SetTo("");

	if ((bytes[0] == B_ENTER) && (modifiers & B_COMMAND_KEY))
		Insert("\n");
	else if (bytes[0] == B_ENTER)
		fConversationView->MessageReceived(new BMessage(APP_CHAT));
	else
		BTextView::KeyDown(bytes, numBytes);
}


void
SendTextView::_AutoComplete()
{
	BStringList words;
	BString text = Text();
	text.Split(" ", true, words);
	if (words.CountStrings() <= 0)
		return;
	BString lastWord = words.StringAt(words.CountStrings() - 1);

	if (fCurrentWord.IsEmpty() == true)
		fCurrentWord = lastWord;

	// No command auto-completion (yet)
	if (fCurrentWord.StartsWith("/") == true)
		return;

	BString user;
	UserMap users = fConversationView->GetConversation()->Users();
	for (int i, j = 0; i < users.CountItems(); i++)
		if (users.KeyAt(i).StartsWith(fCurrentWord)) {
			if (j == fCurrentIndex) {
				user = users.KeyAt(i);
				fCurrentIndex++;
				break;
			}
			j++;
		}

	if (user.IsEmpty() == true)
		fCurrentIndex = 0;
	else {
		text.ReplaceLast(lastWord.String(), user.String());
		SetText(text.String());
		Select(TextLength(), TextLength());
	}
}
