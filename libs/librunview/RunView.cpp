/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RunView.h"

#include <Cursor.h>
#include <Window.h>


RunView::RunView(const char* name)
	:
	BTextView(name),
	fLastStyled(false),
	fUrlCursor(new BCursor(B_CURSOR_ID_FOLLOW_LINK)),
	fMouseDown(false),
	fSelecting(false)
{
	MakeEditable(false);
	SetStylable(true);
	AdoptSystemColors();
	SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);

	text_run run = { 0, BFont(), ui_color(B_PANEL_TEXT_COLOR) };
	fDefaultRun = { 1, {run} };

	BFont urlFont;
	urlFont.SetFace(B_UNDERSCORE_FACE);
	text_run urlRun = { 0, urlFont, ui_color(B_LINK_TEXT_COLOR) };
	fUrlRun = { 1, {urlRun} };
}


void
RunView::Append(const char* text, rgb_color color, uint16 fontFace)
{
	BFont font;
	font.SetFace(fontFace);
	text_run run = { 0, font, color };
	text_run_array array = { 1, {run} };

	Insert(text, &array);
	fLastStyled = true;
}


void
RunView::Append(const char* text)
{
	if (fLastStyled == false)
		Insert(text);
	else
		Insert(text, &fDefaultRun);
	fLastStyled = false;
}


void
RunView::Insert(const char* text, const text_run_array* runs)
{
	BString buf(text);

	int32 urlOffset  = 0;
	int32 lastEnd = 0;
	while (lastEnd < buf.CountChars() && urlOffset != B_ERROR)
	{
		urlOffset = buf.FindFirst("://", lastEnd);

		int32 urlStart = buf.FindLast(" ", urlOffset) + 1;
		int32 urlEnd = buf.FindFirst(" ", urlOffset);
		if (urlStart == B_ERROR || urlOffset == B_ERROR)
			urlStart = lastEnd;
		if (urlEnd == B_ERROR || urlOffset == B_ERROR)
			urlEnd = buf.CountChars();

		BString url;
		BString nonurl;
		if (urlOffset == B_ERROR)
			buf.CopyCharsInto(nonurl, urlStart, urlEnd - urlStart);
		else {
			buf.CopyCharsInto(nonurl, lastEnd, urlStart - lastEnd);
			buf.CopyCharsInto(url, urlStart, urlEnd - urlStart);
		}

		// Actually insert the text
		if (nonurl.IsEmpty() == false)
			BTextView::Insert(nonurl.String(), runs);
		if (url.IsEmpty() == false)
			BTextView::Insert(url.String(), &fUrlRun);

		lastEnd = urlEnd;
	}
}


void
RunView::MouseDown(BPoint where)
{
	uint32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32*)&buttons);

	if (!(buttons & B_SECONDARY_MOUSE_BUTTON))
		BTextView::MouseDown(where);

	BUrl url;
	if ((buttons & B_PRIMARY_MOUSE_BUTTON) && OverUrl(where, &url)) {
		fMouseDown = true;
		if (url.IsValid() == true)
			fLastClicked = url;
	}
}


void
RunView::MouseUp(BPoint where)
{
	BTextView::MouseUp(where);

	if (fMouseDown && fSelecting == false && fLastClicked.IsValid() == true) {
		fLastClicked.OpenWithPreferredApplication(true);
		fLastClicked = BUrl();
	}
	fMouseDown = false;
	fSelecting = false;
}


void
RunView::MouseMoved(BPoint where, uint32 code, const BMessage* drag)
{
	if (fSelecting == true)
		return;

	if (code == B_INSIDE_VIEW)
		if (OverUrl(where) == true)
			SetViewCursor(fUrlCursor);
		else
			SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);
}


void
RunView::Select(int32 startOffset, int32 endOffset)
{
	BTextView::Select(startOffset, endOffset);
	if (startOffset < endOffset) {
		fSelecting = true;
	}
}


BString
RunView::WordAt(BPoint point)
{
	int32 wordOffset = OffsetAt(point);
	int32 lineOffset = OffsetAt(LineAt(point));
	const char* lineBuff = GetLine(LineAt(point));
	BString line(lineBuff);
	delete lineBuff;

	int32 wordStart = line.FindLast(" ", wordOffset - lineOffset) + 1;
	int32 wordEnd = line.FindFirst(" ", wordOffset - lineOffset);

	if (wordStart == B_ERROR)
		wordStart = 0;
	if (wordEnd == B_ERROR)
		wordEnd = line.CountChars();

	BString buffer;
	line.CopyCharsInto(buffer, wordStart, wordEnd - wordStart);
	return buffer;
}


const char*
RunView::GetLine(int32 line)
{
	int32 length = 0; 
	int32 startOffset = OffsetAt(line);
	int32 maxLength = TextLength() - startOffset;
	while (length < maxLength && ByteAt(startOffset + length) != '\n')
		length++;

	char* buffer = new char[length];
	GetText(startOffset, length, buffer);
	return buffer;
}


bool
RunView::OverText(BPoint where)
{
	int32 offset = OffsetAt(where);
	BPoint point = PointAt(offset);
	if (point.x + 10 < where.x)
		return false;
	return true;
}


bool
RunView::OverUrl(BPoint where, BUrl* url)
{
	if (OverText(where) == false)
		return false;

	BString word = WordAt(where);
	if (BUrl(word.String()).IsValid() == true) {
		if (url != NULL)
			url->SetUrlString(word);
		return true;
	}
	return false;
}


void
RunView::ScrollToBottom()
{
	ScrollToOffset(TextLength());
}
