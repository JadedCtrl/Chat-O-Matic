/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "RenderView.h"

#include <InterfaceDefs.h>


RenderView::RenderView(const char* name)
	:
	RunView(name)
{
}


void
RenderView::AppendMessage(const char* nick, const char* message,
	rgb_color nameColor, time_t time)
{
	if (BString(message).IsEmpty() == true)	return;

	AppendTimestamp(time);
	Append("<", nameColor);
	Append(nick);
	Append("> ", nameColor);
	Append(message);

	if (BString(message).EndsWith("\n") == false)	Append("\n");
}


void
RenderView::AppendGenericMessage(const char* message)
{
	if (BString(message).IsEmpty() == true)	return;
	AppendTimestamp(time(NULL));
	Append(message, ui_color(B_PANEL_TEXT_COLOR), B_BOLD_FACE);
	if (BString(message).EndsWith("\n") == false)	Append("\n");
}


void
RenderView::AppendTimestamp(time_t time)
{
	if (time == 0) {
		Append("[xx:xx] ", ui_color(B_LINK_HOVER_COLOR));
		return;
	}
	char timestamp[9] = { '\0' };
	strftime(timestamp, 8, "[%H:%M] ", localtime(&time));
	Append(timestamp, ui_color(B_LINK_HOVER_COLOR));
}
