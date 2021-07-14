/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _RUN_VIEW_H
#define _RUN_VIEW_H

#include <TextView.h>


class RunView : public BTextView {
public:
	RunView(const char* name);

	void	Append(const char* text, rgb_color color,
				uint16 fontFace = B_REGULAR_FACE);
	void	Append(const char* text);

private:
	bool fLastStyled;
	text_run_array fDefaultRun;
};

#endif // _RUN_VIEW_H
