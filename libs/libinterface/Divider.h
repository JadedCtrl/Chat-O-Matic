/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _DIVIDER_H
#define _DIVIDER_H

#include <View.h>

class BMessage;
class BMessenger;

class Divider : public BView {
public:
					    		Divider(const char* name, uint32 flags = 
									B_FRAME_EVENTS | B_WILL_DRAW);
	virtual						~Divider();

					    		Divider(BMessage* archive);

	virtual		void			Draw(BRect updateRect);
	virtual		void			GetPreferredSize(float* width, float* height);

				status_t		Archive(BMessage* archive, bool deep = true) const;

	static		BArchivable*	Instantiate(BMessage* archive);
 
		    	orientation		Orientation();
    			void			Orientation(orientation orient);
 
private:
				orientation		fOrient;
};

#endif	// _DIVIDER_H
