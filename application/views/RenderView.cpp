/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RenderView.h"

#include <InterfaceDefs.h>


RenderView::RenderView(const char* name)
	:
	RunView(name),
	fLastDay(364),
	fLastYear(64)
{
}


void
RenderView::AppendGeneric(const char* message)
{
	if (BString(message).IsEmpty() == true)	return;
	AppendTimestamp(time(NULL));
	Append(message, ui_color(B_PANEL_TEXT_COLOR), B_BOLD_FACE);
	if (BString(message).EndsWith("\n") == false)	Append("\n");
}


void
RenderView::AppendUserstamp(const char* nick, rgb_color nameColor)
{
	Append("<", nameColor, B_BOLD_FACE);
	Append(nick, nameColor, B_BOLD_FACE);
	Append("> ", nameColor, B_BOLD_FACE);
}


void
RenderView::AppendTimestamp(time_t time)
{
	tm* tm = localtime(&time);

	// If day changed, print date divider
	if (fLastDay < tm->tm_yday || fLastYear < tm->tm_year) {
		char datestamp[11] = { '\0' };
		strftime(datestamp, 10, "%Y-%m-%d", tm);
		BString stamp("――― %date% ―――\n");
		stamp.ReplaceAll("%date%", datestamp);

		Append(stamp.String(), ui_color(B_PANEL_TEXT_COLOR),
			B_ITALIC_FACE | B_BOLD_FACE);

		fLastDay = tm->tm_yday;
		fLastYear = tm->tm_year;
	}

	if (time == 0) {
		Append("[xx:xx] ", ui_color(B_LINK_ACTIVE_COLOR), B_BOLD_FACE);
		return;
	}
	char timestamp[9] = { '\0' };
	strftime(timestamp, 8, "[%H:%M] ", tm);
	Append(timestamp, ui_color(B_LINK_ACTIVE_COLOR), B_BOLD_FACE);
}
