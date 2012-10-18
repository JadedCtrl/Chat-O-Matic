/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Vision.
 *
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 *
 * Contributor(s): Rene Gollent
 *                 Todd Lair
 *				   Andrea Anzani, andrea.anzani@gmail.com
 */

#define NUMBER_THEME_READERS  1000

#include <Message.h>
#include <Messenger.h>
#include <View.h>
#include <malloc.h>
#include <stdio.h> //fix
#include <string.h>

#include "Theme.h"
#include "NormalTextRender.h"

int Theme::TimestampFore      = 0;
int Theme::TimestampBack      = 0;
int Theme::TimestampFont      = 0;
int Theme::TimespaceFore      = 1;
int Theme::TimespaceBack      = 1;
int Theme::TimespaceFont      = 1;
int Theme::NormalFore         = 2;
int Theme::NormalBack         = 2;
int Theme::NormalFont         = 2;
int Theme::SelectionBack      = 3;

//at least we use a 'normal' text render


Theme::Theme(const char* n, int foreCount, int backCount, int renderCount)
	:
	name (NULL),
	fores (NULL),
	backs (NULL),
	text_renders (NULL),
	fore_count (max_c (foreCount, 4)),
	back_count (max_c (backCount, 4)),
	render_count (max_c (renderCount, 4))
{

	fSoftLineIndent = (float)(MARGIN_WIDTH / 2.0);
	fTextMargin = (float)(MARGIN_WIDTH / 2.0);

	name = strcpy (new char [strlen (n) + 1], n);

	fores = new rgb_color [fore_count];
	backs = new rgb_color [back_count];

	normal_textrender = new NormalTextRender(be_plain_font);

	text_renders = (TextRender**)malloc(render_count * sizeof(TextRender*));
	for ( int i = 0; i < render_count; i++ )
		text_renders[i] = normal_textrender;



	sid = create_sem (NUMBER_THEME_READERS, name);

	rgb_color def_timestamp_fore  = {200, 150, 150, 255};
	rgb_color def_timestamp_back  = {255, 255, 255, 255};
	rgb_color def_fore            = {0, 0, 0, 255};
	rgb_color def_back            = {255, 255, 255, 255};

	fores[0] = def_timestamp_fore;

	int i;
	for (i = 1; i < fore_count; ++i)
		fores[i] = def_fore;

	backs[0] = def_timestamp_back;
	for (i = 1; i < back_count; ++i)
		backs[i] = def_back;
}


Theme::~Theme (void)
{
	delete_sem (sid);

	//delete [] fonts;
	for ( int i = 0; i < render_count; i++ )
		if ( text_renders[i] )
			text_renders[i] = NULL;

	delete normal_textrender;

	delete [] backs;
	delete [] fores;
	delete [] name;
}


int
Theme::CountForegrounds (void) const
{
	return fore_count;
}


int
Theme::CountBackgrounds (void) const
{
	return back_count;
}
/*
int16
Theme::CountFonts (void) const
{
  return font_count;
}
*/


int
Theme::CountTextRenders (void) const
{
	return render_count;
}


void
Theme::ReadLock (void)
{
	acquire_sem (sid);
}


void
Theme::ReadUnlock (void)
{
	release_sem (sid);
}


void
Theme::WriteLock (void)
{
	acquire_sem_etc (sid, NUMBER_THEME_READERS, 0, 0);
}


void
Theme::WriteUnlock (void)
{
	release_sem_etc (sid, NUMBER_THEME_READERS, 0);
}


const rgb_color
Theme::ForegroundAt (int which) const
{
	rgb_color color = {0, 0, 0, 255};

	if (which >= fore_count || which < 0)
		return color;

	return fores[which];
}


const rgb_color
Theme::BackgroundAt (int which) const
{
	rgb_color color = {255, 255, 255, 255};

	if (which >= back_count || which < 0)
		return color;

	return backs[which];
}
/*
const BFont &
Theme::FontAt (int16 which) const
{
  if (which >= font_count || which < 0)
    return *be_plain_font;

  return fonts[which];
}
*/


TextRender*
Theme::TextRenderAt (int which)
{
	if ( which < 0 ) {
		//printf("Theme::TextRenderAt(): which < 0 (%d)\n", which);
		return normal_textrender;
	}
	if ( which >= render_count ) {
		//printf("Theme::TextRenderAt(): which >= render_count (%d, %d)\n", which, render_count);
		return normal_textrender;
	}

	return text_renders[which];
}


bool
Theme::SetForeground (int which, const rgb_color color)
{
	if (which >= fore_count || which < 0)
		return false;

	fores[which] = color;
	return true;
}


bool
Theme::SetBackground (int which, const rgb_color color)
{
	if (which >= back_count || which < 0)
		return false;

	backs[which] = color;
	return true;
}


bool
Theme::SetTextRender(int which, TextRender* trender)
{


	if (which >= render_count || which < 0 || !trender)
		return false;

	text_renders[which] = trender;
	return true;
}


void
Theme::AddView (BView* view)
{
	list.AddItem (view);
}


void
Theme::RemoveView (BView* view)
{
	list.RemoveItem (view);
}


void
Theme::SetTextMargin(float margin)
{
	fTextMargin = margin;
}


void
Theme::SetSoftLineIndent(float indent)
{
	fSoftLineIndent = indent;
}
