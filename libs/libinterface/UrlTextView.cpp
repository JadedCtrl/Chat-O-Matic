/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "UrlTextView.h"

#include <ctype.h>

#include <Cursor.h>
#include <Locale.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <TextView.h>
#include <Window.h>


const uint32 kSearchDdg = 'RVse';
const uint32 kSearchDict = 'RVdc';


UrlTextView::UrlTextView(const char* name)
	:
	UrlTextView(name, NULL, NULL, B_WILL_DRAW)
{
}


UrlTextView::UrlTextView(const char* name, const BFont* initialFont,
	const rgb_color* initialColor, uint32 flags)
	:
	BTextView(name, initialFont, initialColor, flags),
	fUrlCursor(new BCursor(B_CURSOR_ID_FOLLOW_LINK)),
	fMouseDown(false),
	fSelecting(false)
{
	MakeEditable(false);
	SetStylable(true);
	AdoptSystemColors();
	SetViewCursor(B_CURSOR_SYSTEM_DEFAULT);

	BFont urlFont;
	urlFont.SetFace(B_REGULAR_FACE | B_UNDERSCORE_FACE);
	text_run urlRun = { 0, urlFont, ui_color(B_LINK_TEXT_COLOR) };
	fUrlRun = { 1, {urlRun} };
}


void
UrlTextView::MessageReceived(BMessage* msg)
{
	switch (msg->what)
	{
		case kSearchDdg:
		case kSearchDict:
		{
			int32 start = 0;
			int32 end = 0;
			GetSelection(&start, &end);
			if (start == end)
				break;
			char* buffer = new char[end - start];
			GetText(start, end - start, buffer);

			// Build query
			BString query;
			if (msg->what == kSearchDict)
				query = "https://%lang%.wiktionary.org/w/index.php?search=%q%";
			else
				query = "https://duckduckgo.com/?q=%q%";

			BLanguage lang;
			if (BLocale().GetLanguage(&lang) == B_OK)
				query.ReplaceAll("%lang%", lang.Code());
			else
				query.ReplaceAll("%lang", "eo");

			query.ReplaceAll("%q%", BUrl::UrlEncode(BString(buffer)));

			// Send query
			BUrl url(query.String());
			if (url.IsValid())
				url.OpenWithPreferredApplication(true);
			break;
		}
		default:
			BTextView::MessageReceived(msg);
	}
}


void
UrlTextView::MouseDown(BPoint where)
{
	uint32 buttons = 0;
	Window()->CurrentMessage()->FindInt32("buttons", (int32*)&buttons);

	if (buttons & B_SECONDARY_MOUSE_BUTTON)
		_RightClickPopUp(where)->Go(ConvertToScreen(where), true, false);
	else
		BTextView::MouseDown(where);

	if ((buttons & B_PRIMARY_MOUSE_BUTTON) && OverUrl(where)) {
		fMouseDown = true;

		BUrl url = UrlAt(where);
		if (url.IsValid() == true) {
			fLastClicked = url;
		}
	}
}


void
UrlTextView::MouseUp(BPoint where)
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
UrlTextView::MouseMoved(BPoint where, uint32 code, const BMessage* drag)
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
UrlTextView::Select(int32 startOffset, int32 endOffset)
{
	BTextView::Select(startOffset, endOffset);
	if (startOffset < endOffset) {
		fSelecting = true;
	}
}


void
UrlTextView::Insert(const char* text, const text_run_array* runs)
{
	BString buf(text);

	int32 specStart = 0;
	int32 specEnd = 0;
	int32 lastEnd = 0;
	int32 length = buf.CountChars();

	while (_FindUrlString(buf, &specStart, &specEnd, lastEnd) == true) {
		if (lastEnd < specStart) {
			BString normie;
			buf.CopyCharsInto(normie, lastEnd, specStart - lastEnd);
			BTextView::Insert(TextLength(), normie.String(), normie.Length(),
				runs);
		}
		BString special;
		buf.CopyCharsInto(special, specStart, specEnd - specStart);
		BTextView::Insert(TextLength(), special.String(), special.Length(),
			&fUrlRun);

		lastEnd = specEnd;
	}
	if (lastEnd < length) {
		BString remaining;
		buf.CopyCharsInto(remaining, lastEnd, length - lastEnd);
		BTextView::Insert(TextLength(), remaining.String(), remaining.Length(),
			runs);
	}
}


void
UrlTextView::SetText(const char* text, const text_run_array* runs)
{
	BTextView::SetText("");
	Insert(text, runs);
}


BString
UrlTextView::WordAt(BPoint point)
{
	int32 start = 0;
	int32 end = 0;
	BString word;

	FindWordAround(OffsetAt(point), &start, &end, &word);
	return word;
}


void
UrlTextView::FindWordAround(int32 offset, int32* start, int32* end, BString* _word)
{
	int32 lineOffset = OffsetAt(LineAt(offset));
	const char* lineBuff = GetLine(LineAt(offset));
	BString line(lineBuff);
	delete lineBuff;

	int32 wordStart = line.FindLast(" ", offset - lineOffset) + 1;
	int32 wordEnd = line.FindFirst(" ", offset - lineOffset);

	if (wordStart == B_ERROR)
		wordStart = 0;
	if (wordEnd == B_ERROR)
		wordEnd = line.CountChars();

	*start = lineOffset + wordStart;
	*end = lineOffset + wordEnd;

	if (_word != NULL)
		line.CopyCharsInto(*_word, wordStart, wordEnd - wordStart);
}


const char*
UrlTextView::GetLine(int32 line)
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


BUrl
UrlTextView::UrlAt(BPoint where)
{
	BString urlStr, line = GetLine(LineAt(where));
	BUrl url;

	int32 start;
	int32 end;
	if (_FindUrlString(line, &start, &end, 0) == true) {
		line.CopyCharsInto(urlStr, start, end - start);
		url.SetUrlString(urlStr);
	}
	return url;
}


bool
UrlTextView::OverText(BPoint where)
{
	int32 offset = OffsetAt(where);
	BPoint point = PointAt(offset);
	if (point.x + 10 < where.x)
		return false;
	return true;
}


bool
UrlTextView::OverUrl(BPoint where)
{
	if (OverText(where) == false)
		return false;

	int32 offset = OffsetAt(where);
	text_run_array* rArray = RunArray(offset, offset + 1);
	text_run run = rArray->runs[0];
	text_run urlRun = fUrlRun.runs[0];

	if (run.font.Face() == urlRun.font.Face()
			&& (run.color == urlRun.color))
		return true;
	return false;
}


BPopUpMenu*
UrlTextView::_RightClickPopUp(BPoint where)
{
	BPopUpMenu* menu = new BPopUpMenu("rightClickPopUp");
	BMenuItem* ddgSearch =
		new BMenuItem("Search" B_UTF8_ELLIPSIS, new BMessage(kSearchDdg));
	BMenuItem* dictSearch =
		new BMenuItem("Dictionary" B_UTF8_ELLIPSIS, new BMessage(kSearchDict));
	BMenuItem* copy =
		new BMenuItem("Copy", new BMessage(B_COPY), 'C', B_COMMAND_KEY);
	BMenuItem* selectAll = new BMenuItem("Select all",
		new BMessage(B_SELECT_ALL), 'A', B_COMMAND_KEY);

	// Try and ensure we have something selected
	int32 start = 0;
	int32 end = 0;
	GetSelection(&start, &end);

	if (start == end && OverText(where) == true) {
		start = 0;
		end = 0;
		FindWordAround(OffsetAt(where), &start, &end);
		Select(start, end);
	}
	copy->SetEnabled(start < end);
	dictSearch->SetEnabled(start < end);
	ddgSearch->SetEnabled(start < end);

	menu->AddItem(ddgSearch);
	menu->AddItem(dictSearch);
	menu->AddSeparatorItem();
	menu->AddItem(copy);
	menu->AddItem(selectAll);
	menu->SetTargetForItems(this);
	return menu;
}


bool
UrlTextView::_FindUrlString(BString text, int32* start, int32* end, int32 offset)
{
	int32 urlOffset = text.FindFirst("://", offset);
	int32 urlStart = text.FindLast(" ", urlOffset) + 1;
	int32 urlEnd = text.FindFirst(" ", urlOffset);

	if (urlOffset == B_ERROR)
		return false;

	if (urlStart == B_ERROR)	urlStart = 0;
	if (urlEnd == B_ERROR)		urlEnd = text.CountChars();

	// Find first char of protocol
	for (int32 i = urlStart; i < urlOffset; i++)
		if (_IsValidUrlChar(text.ByteAt(i)) == true) {
			urlStart = i;
			break;
		}

	// Find last char of URL
	for (int32 i = urlOffset; i < urlEnd; i++)
		if (_IsValidUrlChar(text.ByteAt(i)) == false) {
			urlEnd = i;
			break;
		}

	*start = urlStart;
	*end = urlEnd;
	return true;
}


bool
UrlTextView::_IsValidUrlChar(char c)
{
	return (isalpha(c) || isdigit(c) || c == ':' || c == '%' || c == ':'
		|| c == '/' || c == '?' || c == '#' || c == '[' || c == ']' || c == '@'
		|| c == '!' || c == '$' || c == '&' || c == '(' || c == ')' || c == '*'
		|| c == '*' || c == '+' || c == ',' || c == ';' || c == '=' || c == '-'
		|| c == '.' || c == '_' || c == '~' || c == '\'');
}
