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

	// Only differs in that it changes font face and color of any URLs
	virtual void	Insert(const char* text, const text_run_array* runs = NULL);

private:
	bool fLastStyled;
	text_run_array fDefaultRun;
	text_run_array fUrlRun;
};

#endif // _RUN_VIEW_H
