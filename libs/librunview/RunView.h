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

#ifndef RUNVIEW_H_
#define RUNVIEW_H_

#define LINE_COUNT		1000

#include <View.h>

const uint32 M_LOOKUP_WEBSTER = 'rvlw';
const uint32 M_LOOKUP_GOOGLE = 'rvlg';
const uint32 M_LOOKUP_ACRONYM = 'rvla';
const uint32 M_CLEAR = 'rvcl';

const uint32 M_OFFVIEW_SELECTION = 'rvos';
const uint32 M_THEME_FOREGROUND_CHANGE = 'rvtf';
const uint32 M_THEME_BACKGROUND_CHANGE = 'rvtb';
const uint32 M_THEME_FONT_CHANGE = 'rvto';

#define C_URL						0
#define F_URL						0

struct Line;
class Theme;
class RunView;
class BScrollView;
class BCursor;
class BMessageRunner;
class BPopUpMenu;

class SelectPos
{
public:

	int					fLine;
	int					fOffset;

	SelectPos (
	    int selLine = 0,
	    int selOffset = 0)
		:	fLine (selLine),
		    fOffset (selOffset)
	{ }

	SelectPos (const SelectPos& pos)
		:	fLine (pos.fLine),
		    fOffset (pos.fOffset)
	{ }

	~SelectPos (void)
	{ }

	SelectPos&			operator = (const SelectPos& pos) {
		fLine = pos.fLine;
		fOffset = pos.fOffset;

		return *this;
	}

	inline int			operator == (const SelectPos& rhs) const {
		return ((fLine == rhs.fLine) && (fOffset == rhs.fOffset));
	}

	inline int          operator != (const SelectPos& rhs) const {
		return (!(*this == rhs));
	}


};

class RunView : public BView
{
	BScrollView*			fScroller;
	BCursor*				fURLCursor;
	Theme*					fTheme;

	Line*					fWorking;
	Line*					fLines[LINE_COUNT];
	int16					fLine_count,
	                  		fClickCount;

	char*					fStamp_format;
	char*					fClipping_name;

	SelectPos				fSp_start, fSp_end;

	int32					fTracking;
	SelectPos				fTrack_offset;
	BMessageRunner*			fOff_view_runner;
	bigtime_t				fOff_view_time;

	bool 					fResizedirty;
	bool					fFontsdirty;
	BPopUpMenu*				fMyPopUp;
	BPoint					fLastClick;
	bigtime_t				fLastClickTime;


	bool					RecalcScrollBar (bool constrain);
	void					ResizeRecalc (void);
	void					FontChangeRecalc (void);
	void					ExtendTrackingSelect (BPoint);
	void					ShiftTrackingSelect (BPoint, bool, bigtime_t);
	void					CheckURLCursor (BPoint);
	void					BuildPopUp (void);

	bool					CheckClickBounds (const SelectPos&,
								const BPoint&) const;
	void					LoadURL(const char* url);

public:

	RunView (
	    BRect,
	    const char*,
	    Theme*,
	    uint32 = B_FOLLOW_LEFT | B_FOLLOW_TOP,
	    uint32 = 0UL);
	virtual				~RunView (void);

	virtual void			AttachedToWindow (void);
	virtual void			DetachedFromWindow (void);
	virtual void			FrameResized (float, float);
	virtual void			TargetedByScrollView (BScrollView*);
	virtual void			Show ();
	virtual void			Draw (BRect);
	virtual void			MessageReceived (BMessage*);

	virtual void			SetViewColor (rgb_color);
	void					SetViewColor (uchar red, uchar green, uchar blue, uchar alpha = 255) {
		rgb_color color = {red, green, blue, alpha};
		SetViewColor (color);
	}


	virtual void			MouseDown (BPoint);
	virtual void			MouseMoved (BPoint, uint32, const BMessage*);
	virtual void			MouseUp (BPoint);


	void					Append (const char*, int32, int, int, int);
	void					Append (const char*, int, int, int);

	void					Clear (void);

	int16					LineCount (void) const;
	const char*				LineAt (int) const;

	void					SetTimeStampFormat (const char*);
	void					SetTheme (Theme*);

	SelectPos				PositionAt (BPoint) const;
	BPoint					PointAt (SelectPos) const;

	BRect					GetTextFrame (const SelectPos&, const SelectPos&) const;
	bool					IntersectSelection (const SelectPos&, const SelectPos&) const;
	void					GetSelectionText (BString&) const;
	void					Select (const SelectPos&, const SelectPos&);
	void					SelectAll (void);
	void					SetClippingName (const char*);
	bool               		FindText(const char*);

	void					ScrollToBottom(void);
	void					ScrollToSelection(void);



};

#endif
