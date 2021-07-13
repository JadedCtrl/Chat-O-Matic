#include "RenderView.h"

#include <librunview/RunView.h>
#include <librunview/Theme.h>

#include "AppPreferences.h"


RenderView::RenderView(const char *name,  const char* smileyConfig)
	:
	RunView(BRect(0, 0, 1, 1), name,
			fTheme = new Theme(name, COL_MAX_COLORS + 1, COL_MAX_COLORS + 1,
							   MAX_RENDERS + 1),
			B_FOLLOW_ALL, B_WILL_DRAW)
{
	if (smileyConfig)
		Emoticor::Get()->LoadConfig(smileyConfig);

	PrepareTheme(fTheme);

	SetTimeStampFormat("[%H:%M]");
	if ( IsHidden() )
		Show();
	ScrollToBottom();
}


void
RenderView::AppendMessage(const char* nick, const char* message,
							  rgb_color nameColor, time_t time)
{
	rgb_color bg = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color fg = ui_color(B_PANEL_TEXT_COLOR);

	Append("<", nameColor, bg, nameColor, time);
	Append(nick, fg, bg, fg);
	Append("> ", nameColor, bg, nameColor);
	if (Emoticor::Get()->Config() == NULL)
		Append(message, fg, bg, fg);
	else
		AddEmoticText(message, fg, bg);
	Append("\n", fg, bg, fg);
	ScrollToSelection();
}


void
RenderView::AppendGenericMessage(const char* message)
{
	rgb_color bg = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color fg = ui_color(B_PANEL_TEXT_COLOR);

	Append(message, fg, bg, fg, time(NULL));
	ScrollToSelection();
}


void
RenderView::AddEmoticText(const char * txt, rgb_color fore, rgb_color bg)
{
	if (AppPreferences::Item()->IgnoreEmoticons)
		Append(txt, fore, bg, fore);
	else
		Emoticor::Get()->AddText(this, txt, fore, fore, bg, fore);
}


void
RenderView::PrepareTheme(Theme *fTheme)
{
	Theme::TimestampFore = COL_TIMESTAMP_DUMMY;
	Theme::TimestampBack = COL_TIMESTAMP_DUMMY;
	Theme::TimespaceFore = COL_MAX_COLORS;
	Theme::TimespaceBack = COL_MAX_COLORS;
	Theme::TimespaceFont = MAX_RENDERS;
	Theme::TimestampFont = R_TIMESTAMP_DUMMY;
	Theme::NormalFore = COL_TEXT;
	Theme::NormalBack = COL_TEXT;
	Theme::NormalFont = R_TEXT;
	Theme::SelectionBack = COL_SELECTION;

	fTheme->WriteLock();
	rgb_color bg = ui_color(B_PANEL_BACKGROUND_COLOR);
	rgb_color fg = ui_color(B_PANEL_TEXT_COLOR);

	fTheme->SetForeground(COL_URL, ui_color(B_LINK_TEXT_COLOR));
	fTheme->SetBackground(COL_URL, bg);

	fTheme->SetForeground(COL_TIMESTAMP, fg);
	fTheme->SetBackground(COL_TIMESTAMP, bg);

	fTheme->SetForeground(COL_TEXT, fg);
	fTheme->SetBackground(COL_TEXT, bg);

	fTheme->SetForeground(COL_ACTION, fg);
	fTheme->SetBackground(COL_ACTION, bg);

	fTheme->SetForeground(COL_SELECTION, ui_color(B_MENU_SELECTED_ITEM_TEXT_COLOR));
	fTheme->SetBackground(COL_SELECTION, ui_color(B_MENU_SELECTED_BACKGROUND_COLOR));

	fTheme->SetForeground(COL_OWNNICK, 0, 0, 255);
	fTheme->SetBackground(COL_OWNNICK, bg);

	fTheme->SetForeground(COL_OTHERNICK, 255, 0, 0);
	fTheme->SetBackground(COL_OTHERNICK, bg);

	if (Emoticor::Get()->Config() != NULL)
		fTheme->SetTextRender(R_EMOTICON, &str);

	fTheme->SetSoftLineIndent(5.0);
	fTheme->SetTextMargin(5.0);

	fTheme->WriteUnlock();
}


