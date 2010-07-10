/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef		_PercentageWindow_H_
#define	_PercentageWindow_H_

#include <Window.h>
#include <Looper.h>
#include <Bitmap.h>
#include <String.h>
#include <StatusBar.h>

class ReflowingTextView;
class BStringView;


class PercentageWindow : public BWindow
{

public:
	PercentageWindow(const char* title, const char* text, BBitmap* icon = NULL);
	void		Go(BLooper* lop = NULL, int32 msg = 0);
	void		SetPercentage(int perc);
	int		GetPercentage();
	bool		QuitRequested();

private:
	BLooper*		fLooper;
	int32			fMsg;
	float 			kTextIconOffset;
	BStringView*	perc;
	BStatusBar*		pw;

};

#endif
