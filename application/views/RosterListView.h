/*
 * Copyright 2009, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ROSTER_LIST_VIEW_H
#define _ROSTER_LIST_VIEW_H

#include <ListView.h>

class BPopUpMenu;

class RosterItem;

class RosterListView : public BListView
{
public:
					RosterListView(const char* name);

	virtual	void	MessageReceived(BMessage* msg);
	virtual	void	MouseMoved(BPoint where, uint32 code, const BMessage*);
	virtual	void	MouseDown(BPoint where);
	virtual	void	Draw(BRect updateRect);

			void	Sort();

private:
	BPopUpMenu*		fPopUp;
	RosterItem*		fPrevItem;
};

#endif	// _ROSTER_LIST_VIEW_H
