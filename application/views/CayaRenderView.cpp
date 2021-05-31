#include "CayaRenderView.h"

#include <librunview/RunView.h>
#include <librunview/Theme.h>

#include "CayaPreferences.h"


CayaRenderView::CayaRenderView(const char *name,  const char* smileyConfig)
	:
	RunView(BRect(0, 0, 1, 1), name, fTheme = new Theme(name, COL_MAX_COLORS + 1, COL_MAX_COLORS + 1, MAX_RENDERS + 1), B_FOLLOW_ALL, B_WILL_DRAW )
{
	if (smileyConfig)
		Emoticor::Get()->LoadConfig(smileyConfig);

	PrepareTheme(fTheme);

	SetViewColor(245, 245, 245, 0);
	SetLowColor(245, 245, 245, 0);
	SetHighColor(0, 0, 0, 0);


	SetTimeStampFormat(NULL);
	if ( IsHidden() )
		Show();
	ScrollToBottom();
}


void
CayaRenderView::AppendOtherMessage(const char* otherNick, const char* message)
{
	Append(otherNick, COL_OTHERNICK, COL_OTHERNICK, R_TEXT);
	Append(": ", COL_OTHERNICK, COL_OTHERNICK, R_TEXT);
	AddEmoticText(message, COL_TEXT, R_TEXT, COL_TEXT,R_EMOTICON);
	Append("\n", COL_TEXT, COL_TEXT, R_TEXT);
	ScrollToSelection();
}


void
CayaRenderView::AppendOwnMessage(const char* message)
{
	Append("You: ", COL_OWNNICK, COL_OWNNICK, R_TEXT);
	AddEmoticText(message, COL_TEXT, R_TEXT,COL_TEXT,R_EMOTICON);
	Append("\n", COL_TEXT, COL_TEXT, R_TEXT);
	ScrollToSelection();
}


void
CayaRenderView::AppendGenericMessage(const char* message)
{
	Append(message, COL_TEXT, COL_TEXT, R_TEXT);
	ScrollToSelection();
}


void
CayaRenderView::AddEmoticText(const char * txt,  int16 cols , int16 font , int16 cols2 , int16 font2)
{
	if (CayaPreferences::Item()->IgnoreEmoticons)
		Append(txt,cols,cols,font);
	else
		Emoticor::Get()->AddText(this, txt, cols, font, cols2, font2);
}


void
CayaRenderView::PrepareTheme(Theme *fTheme)
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

	fTheme->SetTextRender(R_EMOTICON, &str);

	fTheme->SetSoftLineIndent(5.0);
	fTheme->SetTextMargin(5.0);

	fTheme->WriteUnlock();
}


