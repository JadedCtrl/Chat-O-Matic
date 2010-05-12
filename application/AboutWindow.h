/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2007-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ABOUT_WINDOW_H
#define _ABOUT_WINDOW_H

#include <String.h>

class AboutWindow {
public:
						AboutWindow(const char* appName, const char** holders,
								    const char** authors, const char* extraInfo = NULL);
	virtual				~AboutWindow();

			void		Show();

private:
			BString* 	fAppName;
			BString*	fText;
};

#endif	// _ABOUT_WINDOW_H
