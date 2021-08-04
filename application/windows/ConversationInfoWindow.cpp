/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */

#include "ConversationInfoWindow.h"

#include "Conversation.h"

#include <libinterface/BitmapView.h>

#include <Catalog.h>
#include <LayoutBuilder.h>
#include <StringFormat.h>
#include <StringView.h>
#include <TextView.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "ConversationInfoWindow"


ConversationInfoWindow::ConversationInfoWindow(Conversation* chat)
	:
	BWindow(BRect(200, 200, 300, 400),
		B_TRANSLATE("Room information"), B_FLOATING_WINDOW,
		B_NOT_ZOOMABLE | B_AUTO_UPDATE_SIZE_LIMITS),
	fChat(chat)
{
	_InitInterface();
	CenterOnScreen();

	chat->RegisterObserver(this);
}


ConversationInfoWindow::~ConversationInfoWindow()
{
	fChat->UnregisterObserver(this);
}


void
ConversationInfoWindow::_InitInterface()
{
	fIcon = new BitmapView("roomIcon");
	fIcon->SetBitmap(fChat->IconBitmap());

	fNameLabel = new BStringView("nameLabel", fChat->GetName());
	fNameLabel->SetFont(be_bold_font);

	fIdLabel = new BTextView("idLabel", be_fixed_font, NULL, B_WILL_DRAW);
	fIdLabel->SetWordWrap(false);
	fIdLabel->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
	fIdLabel->MakeEditable(false);
	_SetIdLabel(fChat->GetId());

	fUserCountLabel = new BStringView("userCountLabel", "");
	_SetUserCountLabel(fChat->Users().CountItems());

	// Centering is still my lyfeee
	fNameLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fIdLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));
	fUserCountLabel->SetExplicitAlignment(BAlignment(B_ALIGN_CENTER, B_ALIGN_TOP));


	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 10)
		.SetInsets(B_USE_DEFAULT_SPACING)
		.AddGroup(B_VERTICAL)
			.Add(fNameLabel)
			.Add(fIdLabel)
			.AddGlue()
		.End()
		.AddGroup(B_VERTICAL)
		.AddGroup(B_VERTICAL)
			.Add(fIcon)
			.Add(fUserCountLabel)
		.End()
	.End();
}


void
ConversationInfoWindow::_SetIdLabel(BString id)
{
	fIdLabel->SetText(id);
	fIdLabel->SetExplicitMinSize(
		BSize(be_fixed_font->StringWidth(id) + 5, B_SIZE_UNSET));
}


void
ConversationInfoWindow::_SetUserCountLabel(int32 userCount)
{
	BStringFormat pmFormat(B_TRANSLATE("{0, plural,"
		"=1{One lonely user}"
		"=2{Two partners}"
		"other{# members}}"));

	BString label;
	pmFormat.Format(label, userCount);
	fUserCountLabel->SetText(label);
}
