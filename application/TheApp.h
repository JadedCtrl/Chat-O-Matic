/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _THE_APP_H
#define _THE_APP_H

#include <Application.h>

#include "MainWindow.h"

class TheApp : public BApplication {
public:
				TheApp();

	void		ReadyToRun();

	MainWindow*	GetMainWindow() const;

private:
	MainWindow*	fMainWin;

};

#endif	// _THE_APP_H
