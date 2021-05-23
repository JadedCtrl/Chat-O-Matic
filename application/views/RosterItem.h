/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ROSTER_ITEM_H
#define _ROSTER_ITEM_H

#include <Bitmap.h>
#include <GradientLinear.h>
#include <ListItem.h>
#include <View.h>
#include <String.h>

#include "CayaConstants.h"
#include "Contact.h"
#include "Observer.h"

class RosterItem : public BStringItem, public Observer {
public:
					RosterItem(const char* name, Contact* contact);
					~RosterItem();

	bool			IsVisible() const { return fVisible; }
	void			SetVisible(bool visible);

	void			DrawItem(BView *owner, BRect frame,
							 bool complete = false);

	void			Update(BView *owner, const BFont *font);

	Contact*	GetContact() { return contactLinker;}

	CayaStatus		Status() const { return fStatus; }
	void			SetStatus(CayaStatus status);

	BString			PersonalStatus() const { return fPersonalStatus; }
	void			SetPersonalStatus(BString str) { fPersonalStatus = str; }

	BBitmap*		Bitmap() const { return fBitmap; }
	void			SetBitmap(BBitmap *);

protected:
	void			ObserveString(int32 what, BString str);
	void			ObservePointer(int32 what, void* ptr);
	void			ObserveInteger(int32 what, int32 val);

private:
	Contact*	contactLinker;
	float			fBaselineOffset;
	BString			fPersonalStatus;
	CayaStatus		fStatus;
	BBitmap*		fBitmap;
	bool			fVisible;	
	BGradientLinear	fGradient;
};

#endif	// _ROSTER_ITEM_H
