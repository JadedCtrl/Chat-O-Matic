/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2007-2009, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Ryan Leavengood, leavengood@gmail.com
 */

#include <Alert.h>
#include <Font.h>
#include <String.h>
#include <TextView.h>

#include "AboutWindow.h"


AboutWindow::AboutWindow(const char* appName, const char** holders,
						   const char** authors, const char* extraInfo)
{
	fAppName = new BString(appName);

	// Build the text to display
	int32 i;
	BString text(appName);
	text << "\n\n";
	for (i = 0; holders[i]; i++)
		text << "Copyright " B_UTF8_COPYRIGHT " " << holders[i] << "\n";
	text << "\nWritten by:\n";
	for (int32 i = 0; authors[i]; i++)
		text << "    " << authors[i] << "\n";
	
	// The extra information is optional
	if (extraInfo != NULL)
		text << "\n" << extraInfo << "\n";

	fText = new BString(text);
}


AboutWindow::~AboutWindow()
{
	delete fText;
	delete fAppName;
}


void
AboutWindow::Show()
{
	BAlert* alert = new BAlert("About" B_UTF8_ELLIPSIS, fText->String(), "Close");
	BTextView* view = alert->TextView();
	BFont font;
	view->SetStylable(true);
	view->GetFont(&font);
	font.SetFace(B_BOLD_FACE);
	font.SetSize(font.Size() * 1.7f);
	view->SetFontAndColor(0, fAppName->Length(), &font);
	alert->Go();
}
