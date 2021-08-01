/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _RUN_VIEW_H
#define _RUN_VIEW_H

#include <Url.h>

#include <libinterface/UrlTextView.h>

class BPopUpMenu;


class RunView : public UrlTextView {
public:
	RunView(const char* name);

			void	Append(const char* text, rgb_color color, BFont font);
			void	Append(const char* text, rgb_color color, uint16 fontFace);
			void	Append(const char* text, rgb_color color);
			void	Append(const char* text);

			void	ScrollToBottom();

private:
	// For safe-keeping
	text_run_array fDefaultRun;

	// Whether or not the run was changed from default
	bool fLastStyled;
};

#endif // _RUN_VIEW_H
