/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _URL_TEXT_VIEW_H
#define _URL_TEXT_VIEW_H

#include <TextView.h>
#include <Url.h>

class BPopUpMenu;


/*! TextView that highlights and allows selecting of URLs― also provides a
  * right-click pop-up menu with internet search options */
class UrlTextView : public BTextView {
public:
					UrlTextView(const char* name);
					UrlTextView(const char* name, const BFont* initialFont,
						const rgb_color* initialColor, uint32 flags);
					

	virtual void	MessageReceived(BMessage* msg);

	virtual	void	MouseDown(BPoint where);
	virtual void	MouseUp(BPoint where);
	virtual void	MouseMoved(BPoint where, uint32 code, const BMessage* drag);

	virtual void	Select(int32 startOffset, int32 endOffset);

			// Only differs in that it changes font face and color of any URLs
			void	Insert(const char* text, const text_run_array* runs = NULL);
			void	SetText(const char* text, const text_run_array* runs = NULL);

		 BString	WordAt(BPoint point);
			void	FindWordAround(int32 offset, int32* start, int32* end,
						BString* _word = NULL);
	 const char*	GetLine(int32 line);

			BUrl	UrlAt(BPoint point);

			bool	OverText(BPoint where);
			bool	OverUrl(BPoint where);

private:
	 BPopUpMenu*	_RightClickPopUp(BPoint where);

			bool	_FindUrlString(BString text, int32* start, int32* end,
						int32 offset);

			// Checks if char is allowed in a url, as per rfc3986
			bool	_IsValidUrlChar(char c);

	// For safe-keeping
	text_run_array fNormalRun;
	text_run_array fUrlRun;
	BCursor* fUrlCursor;

	// Information between MouseDown and MouseUp
	BUrl fLastClicked;
	bool fMouseDown;
	bool fSelecting;
};

#endif // _URL_TEXT_VIEW_H
