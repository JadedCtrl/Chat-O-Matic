/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2009, Michael Davidson. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _NOTIFYING_TEXT_VIEW_H
#define _NOTIFYING_TEXT_VIEW_H

#include <Handler.h>
#include <Message.h>
#include <TextView.h>

class BMessenger;

class NotifyingTextView : public BTextView {
public:
							NotifyingTextView(const char* name, uint32 flags
								 = B_WILL_DRAW | B_PULSE_NEEDED);
							~NotifyingTextView();

				void		SetTarget(const BHandler* handler);

				BMessage*	Message() const;
				void		SetMessage(BMessage* msg);

protected:
	virtual 	void		InsertText(const char* text, int32 length,
								int32 offset, const text_run_array* runs = NULL);
	virtual 	void		DeleteText(int32 start, int32 finish);

private:
				BMessenger*		fMessenger;
				BMessage*		fMessage;
};

#endif	// _NOTIFYING_TEXT_VIEW_H
