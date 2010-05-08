/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _TOOL_BUTTON_H
#define _TOOL_BUTTON_H

#include <Control.h>

class BBitmap;
class BPopUpMenu;

class ToolButton : public BControl {
public:
							ToolButton(const char* name, const char* label,
							            BMessage* message,
							            uint32 flags = B_WILL_DRAW | B_NAVIGABLE
							                    | B_FULL_UPDATE_ON_RESIZE);
							ToolButton(const char* label, BMessage* message = NULL);
							ToolButton(BMessage* archive);

	virtual					~ToolButton();

	static	BArchivable*	Instantiate(BMessage* archive);
	virtual	status_t		Archive(BMessage* archive, bool deep) const;

	virtual	BSize			MinSize();
	virtual	BSize			MaxSize();
	virtual	BSize			PreferredSize();

	virtual	void			Draw(BRect updateRect);
	virtual	void			AttachedToWindow();

	virtual	void			MouseDown(BPoint where);
	virtual	void			MouseMoved(BPoint where, uint32 transit,
									   const BMessage* message);
	virtual	void			MouseUp(BPoint where);

	virtual	void			SetLabel(const char* string);

	virtual	void			MessageReceived(BMessage* message);
	virtual	void			WindowActivated(bool active);

	virtual	status_t		Invoke(BMessage* message = NULL);

	virtual	status_t		Perform(perform_code code, void* data);

	virtual	void			InvalidateLayout(bool descendants);

			BBitmap*		Bitmap() const;
			void			SetBitmap(BBitmap* bitmap);

			BPopUpMenu*		Menu() const;
			void			SetMenu(BPopUpMenu* menu);

private:
			BBitmap*		fBitmap;
			BPopUpMenu*		fMenu;
			BSize			fPreferredSize;

			BPoint			_Center(BRect rect);
			BSize			_ValidatePreferredSize();
};

#endif	// _TOOL_BUTTON_H
