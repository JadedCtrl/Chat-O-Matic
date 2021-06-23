/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _THE_APP_H
#define _THE_APP_H

#include <Application.h>

class MainWindow;


int main(int argc, char* argv[]);


class TheApp : public BApplication {
public:
						TheApp();

	virtual	void		ReadyToRun();
	virtual	void		AboutRequested();

	virtual	void		MessageReceived(BMessage* message);

			MainWindow*	GetMainWindow() const;

private:
			MainWindow*	fMainWin;
};

#endif	// _THE_APP_H
