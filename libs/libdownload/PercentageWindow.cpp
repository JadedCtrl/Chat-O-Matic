/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "PercentageWindow.h"

#include "StripeView.h"
#include <Alert.h>

#include <StringView.h>
#include <Messenger.h>

#define DEFAULT_RECT	BRect(0, 0, 310, 75)

static const int kTextIconOffsetSpace = 30;

PercentageWindow::PercentageWindow(const char* title, const char* text, BBitmap* icon) : BWindow(DEFAULT_RECT, title, B_MODAL_WINDOW, B_NOT_RESIZABLE | B_NOT_ZOOMABLE)
{

	// Set up the "_master_" view

	StripeView* masterView = new StripeView(Bounds());
	AddChild(masterView);
	masterView->SetBitmap(icon);

	kTextIconOffset = 0;

	if (masterView->Bitmap())
		kTextIconOffset = masterView->Bitmap()->Bounds().right + kTextIconOffsetSpace;


	//ok, un String
	//il percentage (a 0) con testo percentuale
	float maxW;

	BStringView* text_string = new BStringView(BRect(kTextIconOffset, 6, 0, 0), "_text", text);
	masterView->AddChild(text_string);
	text_string->ResizeToPreferred();
	maxW = text_string->Frame().right + 6;

	BRect rect(text_string->Frame());
	rect.OffsetBy(0, text_string->Bounds().Height() + 6);

	perc = new BStringView(rect, "_percentage", "100%");
	masterView->AddChild(perc);
	perc->ResizeToPreferred();
	if (perc->Frame().right + 6 > maxW)
		maxW = perc->Frame().right + 6;

	perc->SetText("0%");

	maxW += kTextIconOffsetSpace;

	ResizeTo(maxW, Bounds().bottom);

	rect = Bounds();

	rect.top = perc->Frame().bottom + 6;
	rect.left = perc->Frame().left;
	rect.right -= kTextIconOffsetSpace;

	pw = new BStatusBar(rect, "status_bar", NULL, NULL);
	pw->SetMaxValue(100.0);
	masterView->AddChild(pw);
	//	pw->ResizeToPreferred();

	ResizeTo(Bounds().right, pw->Frame().bottom + 5);
	SetLook(B_FLOATING_WINDOW_LOOK);
	MoveTo(BAlert::AlertPosition(Frame().Width(), Frame().Height()));
}

void
PercentageWindow::SetPercentage(int p)
{


	BString text;
	text << p << "%";

	if (Lock()) {
		perc->SetText(text.String());
		pw->SetTo((float)p);
		Unlock();
	}

}

int
PercentageWindow::GetPercentage()
{
	return (int)pw->CurrentValue();
}

bool
PercentageWindow::QuitRequested()
{
	if (fLooper)
		BMessenger(fLooper).SendMessage(fMsg);

	return true;
}


void
PercentageWindow::Go(BLooper* lop, int32 msg)
{
	fLooper = lop;
	fMsg = msg;
	Show();
}
