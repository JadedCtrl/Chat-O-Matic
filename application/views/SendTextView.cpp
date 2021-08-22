/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "SendTextView.h"

#include <StringList.h>
#include <Window.h>

#include "AppMessages.h"
#include "MainWindow.h"
#include "Server.h"
#include "TheApp.h"


SendTextView::SendTextView(const char* name, ConversationView* convView)
	:
	BTextView(name),
	fChatView(convView),
	fCurrentIndex(0),
	fHistoryIndex(0)
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
	// Reset auto-complete state if user typed/sent something other than tab
	fCurrentIndex = 0;
	fCurrentWord.SetTo("");

	for (int i = 0; i < numBytes; i++) {
		if ((bytes[i] == B_UP_ARROW) && (modifiers == 0)) {
			_UpHistory();
			return;
		}
		else if ((bytes[i] == B_DOWN_ARROW) && (modifiers == 0)) {
			_DownHistory();
			return;
		}

		if ((bytes[i] == B_ENTER)
				&& ((modifiers & B_COMMAND_KEY) || (modifiers & B_SHIFT_KEY))) {
			Insert("\n");
			return;
		}
		else if (bytes[i] == B_ENTER) {
			_AppendHistory();
			fChatView->MessageReceived(new BMessage(APP_CHAT));
			return;
		}
	}
	BTextView::KeyDown(bytes, numBytes);
}


void
SendTextView::_AutoComplete()
{
	if (fChatView == NULL || fChatView->GetConversation() == NULL)
		return;

	BStringList words;
	BString text = Text();
	text.Split(" ", true, words);
	if (words.CountStrings() <= 0)
		return;
	BString lastWord = words.StringAt(words.CountStrings() - 1);

	if (fCurrentWord.IsEmpty() == true)
		fCurrentWord = lastWord;

	// Now to find the substitutes
	BString substitution;
	if (fCurrentWord.StartsWith("/") == true) {
		substitution =
			_NextMatch(_CommandNames(), BString(fCurrentWord).RemoveFirst("/"));
		if (substitution.IsEmpty() == false)
			substitution.Prepend("/");
	}
	else
		substitution = _NextMatch(_UserNames(), fCurrentWord);

	// Apply the substitution or jet off
	if (substitution.IsEmpty() == true)
		fCurrentIndex = 0;
	else {
		int32 index = text.FindLast(lastWord);
		int32 newindex = index + substitution.Length();

		Delete(index, index + lastWord.CountChars());
		Insert(index, substitution, substitution.Length());
		Select(newindex, newindex);
	}
}


BString
SendTextView::_NextMatch(BStringList list, BString current)
{
	BString match;
	for (int i = 0, j = 0; i < list.CountStrings(); i++)
		if (list.StringAt(i).StartsWith(current)) {
			if (j == fCurrentIndex) {
				match = list.StringAt(i);
				fCurrentIndex++;
				break;
			}
			j++;
		}
	return match;
}


BStringList
SendTextView::_CommandNames()
{
	if (fCurrentIndex == 0) {
		int64 instance = fChatView->GetConversation()->GetProtocolLooper()->GetInstance();
		BStringList cmdNames;
		CommandMap cmds =
			((TheApp*)be_app)->GetMainWindow()->GetServer()->Commands(instance);

		for (int i = 0; i < cmds.CountItems(); i++)
			cmdNames.Add(cmds.KeyAt(i));
		fCurrentList = cmdNames;
	}
	return fCurrentList;
}


BStringList
SendTextView::_UserNames()
{
	if (fCurrentIndex == 0) {
		BStringList nameAndId;
		UserMap users = fChatView->GetConversation()->Users();

		for (int i = 0; i < users.CountItems(); i++) {
			nameAndId.Add(users.KeyAt(i));
			nameAndId.Add(users.ValueAt(i)->GetName());
		}
		fCurrentList = nameAndId;
	}
	return fCurrentList;
}


void
SendTextView::_AppendHistory()
{
	fHistoryIndex = 0;
	fHistory.Add(BString(Text()), 0);
	if (fHistory.CountStrings() == 21)
		fHistory.Remove(20);
}


void
SendTextView::_UpHistory()
{
	if (fHistoryIndex == 0 && TextLength() > 0)
		_AppendHistory();

	if (fHistoryIndex < fHistory.CountStrings()) {
		fHistoryIndex++;
		SetText(fHistory.StringAt(fHistoryIndex - 1));
		Select(TextLength(), TextLength());
	}
}


void
SendTextView::_DownHistory()
{
	if (fHistoryIndex > 1) {
		fHistoryIndex--;
		SetText(fHistory.StringAt(fHistoryIndex - 1));
		Select(TextLength(), TextLength());
	}
}
