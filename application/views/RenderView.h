/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _RENDER_VIEW_H
#define _RENDER_VIEW_H

#include <librunview/RunView.h>


class RenderView : public RunView {
public:
				RenderView(const char* name);

		void	AppendGeneric(const char* message, int64 when);
		void	AppendUserstamp(const char* nick, rgb_color nameColor);
		void	AppendTimestamp(time_t time = 0);

private:
		int fLastDay;
		int fLastYear;
};

#endif // _RENDER_VIEW_H
