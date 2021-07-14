/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RunView.h"


RunView::RunView(const char* name)
	:
	BTextView(name),
	fLastStyled(false)
{
	MakeEditable(false);
	SetStylable(true);
	AdoptSystemColors();

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
