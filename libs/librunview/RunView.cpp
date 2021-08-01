/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RunView.h"

#include <Cursor.h>
#include <Locale.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <TextView.h>
#include <Window.h>


const uint32 kSearchDdg = 'RVse';
const uint32 kSearchDict = 'RVdc';


RunView::RunView(const char* name)
	:
	UrlTextView(name),
	fLastStyled(false)
{
	text_run run = { 0, BFont(), ui_color(B_PANEL_TEXT_COLOR) };
	fDefaultRun = { 1, {run} };
}


void
RunView::Append(const char* text, rgb_color color, BFont font)
{
	text_run run = { 0, font, color };
	text_run_array array = { 1, {run} };

	Insert(text, &array);
	fLastStyled = true;
}


void
RunView::Append(const char* text, rgb_color color, uint16 fontFace)
{
	BFont font;
	font.SetFace(fontFace);
	Append(text, color, font);
}


void
RunView::Append(const char* text, rgb_color color)
{
	Append(text, color, BFont());
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
RunView::ScrollToBottom()
{
	ScrollToOffset(TextLength());
}
