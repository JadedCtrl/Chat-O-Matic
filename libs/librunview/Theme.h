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
 */

#ifndef THEME_H_
#define THEME_H_

#include <OS.h>
#include <GraphicsDefs.h>
#include <List.h>

#include "TextRender.h"

class BView;
class NormalTextRender;

#define MARGIN_WIDTH	10.0 //default value, double of fTextMargin
#define MARGIN_INDENT	10.0 //default value, double of fSoftLineIndent

class Theme
{
	char*				name;
	rgb_color*			fores;
	rgb_color*			backs;
	TextRender**          text_renders; //FIX!!

	int16				fore_count;
	int16				back_count;
	int16				render_count;

	BList				list;
	sem_id				sid;

	float				fSoftLineIndent;
	float				fTextMargin;
	NormalTextRender*	normal_textrender;

public:

	static int16		TimestampFore;
	static int16		TimestampBack;
	static int16		TimestampFont;
	static int16		TimespaceFore;
	static int16		TimespaceBack;
	static int16		TimespaceFont;
	static int16		NormalFore;
	static int16		NormalBack;
	static int16		NormalFont;
	static int16		SelectionBack;

	Theme (
	    const char*,
	    int16,
	    int16,
	    int16);
	virtual				~Theme (void);

	const char*			Name (void) const {
		return name;
	}

	void					ReadLock (void);
	void					ReadUnlock (void);
	void					WriteLock (void);
	void					WriteUnlock (void);

	int16					CountForegrounds (void) const;
	int16					CountBackgrounds (void) const;
	//	int16					CountFonts (void) const;
	int16					CountTextRenders (void) const;

	const rgb_color	ForegroundAt (int16) const;
	const rgb_color	BackgroundAt (int16) const;

	//const BFont			&FontAt (int16) const;

	TextRender*             TextRenderAt(int16);

	bool					SetForeground (int16, const rgb_color);
	bool					SetForeground (int16 w, uchar r, uchar g, uchar b, uchar a = 255) {
		rgb_color color = {r, g, b, a};
		return SetForeground (w, color);
	}
	bool					SetBackground (int16, const rgb_color);
	bool					SetBackground (int16 w, uchar r, uchar g, uchar b, uchar a = 255) {
		rgb_color color = {r, g, b, a};
		return SetBackground (w, color);
	}

	//bool					SetFont (int16, const BFont &);
	bool                    SetTextRender(int16, TextRender*);

	void					SetSoftLineIndent(float indent);
	void					SetTextMargin(float margin);

	float					TextMargin() const {
		return fTextMargin;
	}
	float					SoftLineIndent() const {
		return fSoftLineIndent;
	}



	void					AddView (BView*);
	void					RemoveView (BView*);


};

#endif
