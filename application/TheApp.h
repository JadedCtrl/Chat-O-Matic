/*
 * Copyright 2009-2010, Andrea Anzani. All rights reserved.
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _THE_APP_H
#define _THE_APP_H

#include <Application.h>

class MainWindow;

class TheApp : public BApplication {
public:
						TheApp();

	virtual	void		ReadyToRun();
	virtual	void		AboutRequested();

			MainWindow*	GetMainWindow() const;

private:
			MainWindow*	fMainWin;
};

#endif	// _THE_APP_H
