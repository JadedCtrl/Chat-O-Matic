/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _SEND_TEXT_VIEW_H
#define _SEND_TEXT_VIEW_H

#include <StringList.h>
#include <TextView.h>

#include "ConversationView.h"


class SendTextView : public BTextView {
public:
					SendTextView(const char* name, ConversationView* convView);

	virtual void	KeyDown(const char* bytes, int32 numBytes);

private:
			void	_AutoComplete();
		 BString	_NextMatch(BStringList list, BString current);

	  BStringList	_CommandNames();
	  BStringList	_UserNames();

			void	_AppendHistory();
			void	_UpHistory();
			void	_DownHistory();

	ConversationView* fChatView;

	// Used for auto-completion
	int32 fCurrentIndex;
	BString fCurrentWord;
	BStringList fCurrentList;

	// Used for history
	BStringList fHistory;
	int32 fHistoryIndex;
};

#endif // _SEND_TEXT_VIEW_H
