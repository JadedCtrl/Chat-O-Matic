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

		void	AppendMessage(const char* nick, const char* message,
							  rgb_color nameColor, time_t time = 0);
		void	AppendGeneric(const char* message);
		void	AppendTimestamp(time_t time = 0);

private:
		int fLastDay;
		int fLastYear;
};

#endif // _RENDER_VIEW_H
