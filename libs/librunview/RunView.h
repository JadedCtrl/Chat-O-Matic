/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _RUN_VIEW_H
#define _RUN_VIEW_H

#include <TextView.h>
#include <Url.h>

class BPopUpMenu;


class RunView : public BTextView {
public:
	RunView(const char* name);

	virtual void	MessageReceived(BMessage* msg);

	// Only differs in that it changes font face and color of any URLs
	virtual void	Insert(const char* text, const text_run_array* runs = NULL);

	virtual	void	MouseDown(BPoint where);
	virtual void	MouseUp(BPoint where);
	virtual void	MouseMoved(BPoint where, uint32 code, const BMessage* drag);

	virtual void	Select(int32 startOffset, int32 endOffset);

			void	Append(const char* text, rgb_color color, BFont font);
			void	Append(const char* text, rgb_color color, uint16 fontFace);
			void	Append(const char* text, rgb_color color);
			void	Append(const char* text);

		 BString	WordAt(BPoint point);
			void	FindWordAround(int32 offset, int32* start, int32* end,
						BString* _word = NULL);
	 const char*	GetLine(int32 line);

			bool	OverText(BPoint where);
			bool	OverUrl(BPoint where);

			void	ScrollToBottom();

private:
	 BPopUpMenu*	_RightClickPopUp(BPoint where);

			bool	_FindUrlString(BString text, int32* start, int32* end,
						int32 offset);

	// For safe-keeping
	text_run_array fDefaultRun;
	text_run_array fUrlRun;
	text_run_array fUrlHoverRun;
	text_run_array fUrlVisitedRun;
	BCursor* fUrlCursor;

	// Whether or not the run was changed from default
	bool fLastStyled;

	// Information between MouseDown and MouseUp
	BUrl fLastClicked;
	bool fMouseDown;
	bool fSelecting;
};

#endif // _RUN_VIEW_H
