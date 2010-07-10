/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "ReflowingTextView.h"

const bigtime_t RTV_HIDE_DELAY = 1500000;
const bigtime_t RTV_SHOW_DELAY = 750000;

enum {
	RTV_SHOW_TOOLTIP = 0x1000,
	RTV_HIDE_TOOLTIP
};

ReflowingTextView::ReflowingTextView(BRect frame, const char* name, BRect textRect, uint32 resizeMask, uint32 flags) : BTextView(frame, name, textRect, resizeMask, flags), _showTipRunner(NULL), _canShow(false), _urlTip(NULL), _currentLinkStart(-1)
{
	// empty
}

ReflowingTextView::~ReflowingTextView()
{
	if ((_urlTip) && (_urlTip->Lock())) _urlTip->Quit();
	delete _showTipRunner;
}

void ReflowingTextView::AttachedToWindow()
{
	BTextView::AttachedToWindow();
	//  FixTextRect();
}

void ReflowingTextView::FrameResized(float w, float h)
{
	BTextView::FrameResized(w, h);
	//  FixTextRect();
}

void ReflowingTextView::MouseMoved(BPoint where, uint32 code, const BMessage* msg)
{
	const URLLink* link = NULL;
	if (code == B_INSIDE_VIEW) {
		link = GetURLAt(where);
#ifdef B_BEOS_VERSION_5
		SetViewCursor((link != NULL) ? B_CURSOR_SYSTEM_DEFAULT : B_CURSOR_I_BEAM);
#else
		be_app->SetCursor((link != NULL) ? B_HAND_CURSOR : B_I_BEAM_CURSOR);
#endif
	}

	BString scratch;
	const BString* urlString = link ? &link->GetURL() : &scratch;
	if ((link) && (urlString->Length() == 0)) {
		char* buf = new char[link->GetLength()+1];
		GetText(link->GetStart(), link->GetLength(), buf);
		buf[link->GetLength()] = '\0';
		scratch = buf;
		delete [] buf;
		urlString = &scratch;
	}

	int32 linkStart = link ? (int32)link->GetStart() : -1;
	if (linkStart != _currentLinkStart) {
		_currentLinkStart = linkStart;
		_currentURLString = *urlString;
		if (linkStart >= 0) {
			if ((_canShow == false) && ((_showTipRunner == NULL) || (_runnerWillHide))) {
				// Schedule unsetting the show flag
				delete _showTipRunner;
				_runnerWillHide = false;
				_showTipRunner = new BMessageRunner(this, new BMessage(RTV_SHOW_TOOLTIP), RTV_SHOW_DELAY, 1);
			}
		} else {
			if (_canShow) {
				// schedule a delayed show
				delete _showTipRunner;
				_runnerWillHide = true;
				_showTipRunner = new BMessageRunner(this, new BMessage(RTV_HIDE_TOOLTIP), RTV_HIDE_DELAY, 1);
			} else if ((_showTipRunner) && (_runnerWillHide == false)) {
				delete _showTipRunner;  // cancel the pending show!
				_showTipRunner = NULL;
			}
		}
		UpdateToolTip();
	}
	BTextView::MouseMoved(where, code, msg);
}

void ReflowingTextView::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case RTV_SHOW_TOOLTIP:
			delete _showTipRunner;
			_showTipRunner = NULL;
			_canShow = true;
			UpdateToolTip();
			break;

		case RTV_HIDE_TOOLTIP:
			delete _showTipRunner;
			_showTipRunner = NULL;
			_canShow = false;
			UpdateToolTip();
			break;

		default:
			BTextView::MessageReceived(msg);
			break;
	}
}

void ReflowingTextView::UpdateToolTip()
{
	if ((_canShow) && (_currentLinkStart >= 0)) {
		if (_urlTip == NULL) {
			_urlTip = new BWindow(BRect(0, 0, 5, 5), NULL, B_NO_BORDER_WINDOW_LOOK, B_FLOATING_ALL_WINDOW_FEEL, B_NOT_MOVABLE | B_NOT_CLOSABLE | B_NOT_ZOOMABLE | B_NOT_MINIMIZABLE | B_AVOID_FOCUS);
			BView* blackBorder = new BView(_urlTip->Bounds(), NULL, B_FOLLOW_ALL_SIDES, 0);
			const rgb_color black = {0, 0, 0, 255};
			blackBorder->SetViewColor(black);
			_urlTip->AddChild(blackBorder);

			const rgb_color hiliteYellow = {255, 255, 200, 255};
			BRect inset = blackBorder->Bounds();
			inset.InsetBy(1, 1);
			BView* yellowInterior = new BView(inset, NULL, B_FOLLOW_ALL_SIDES, 0);
			yellowInterior->SetLowColor(hiliteYellow);
			yellowInterior->SetViewColor(hiliteYellow);
			blackBorder->AddChild(yellowInterior);

			_urlStringView = new BStringView(yellowInterior->Bounds(), NULL, "", B_FOLLOW_ALL_SIDES);
			_urlStringView->SetLowColor(hiliteYellow);
			_urlStringView->SetViewColor(hiliteYellow);
			yellowInterior->AddChild(_urlStringView);
		}
		if (_urlTip->Lock()) {
			font_height ttfh;
			_urlStringView->GetFontHeight(&ttfh);
			float urlHeight = 20.0f;
			BPoint pt = PointAt(_currentLinkStart, &urlHeight);
			_urlTip->MoveTo(ConvertToScreen(pt) + BPoint(15.0f, -(ttfh.ascent + ttfh.descent + 3.0f)));

			_urlTip->ResizeTo(muscleMin(_urlStringView->StringWidth(_currentURLString.String()) + 4.0f, 400.0f), ttfh.ascent + ttfh.descent + 2.0f);
			_urlStringView->SetText(_currentURLString.String());
			_urlTip->SetWorkspaces(B_CURRENT_WORKSPACE);
			if (_urlTip->IsHidden()) _urlTip->Show();
			_urlTip->Unlock();
		}
	} else if (_urlTip) {
		if (_urlTip->Lock()) {
			if (_urlTip->IsHidden() == false) _urlTip->Hide();
			_urlTip->Unlock();
		}
	}
}

void ReflowingTextView::MouseDown(BPoint where)
{
	if (IsFocus() == false) MakeFocus();
	const URLLink* url = GetURLAt(where);
	if (url) {
		char* allocBuf = NULL;
		const char* buf = url->GetURL().String();
		if (buf[0] == '\0') {
			buf = allocBuf = new char[url->GetLength()+1];
			GetText(url->GetStart(), url->GetLength(), allocBuf);
			allocBuf[url->GetLength()] = '\0';
		}

		if (strncasecmp(buf, "beshare://", 10) == 0) {
			char* argv[] = {(char*) &buf[10]};
			// be_roster->Launch(BESHARE_MIME_TYPE, ARRAYITEMS(argv), argv);
		} else if (strncasecmp(buf, "beshare:", 8) == 0) {
			BMessage msg(_queryMessage);
			msg.AddString("query", &buf[8]);
			_commandTarget.SendMessage(&msg);
		} else if (strncasecmp(buf, "share:", 6) == 0) {
			BMessage msg(_queryMessage);
			msg.AddString("query", &buf[6]);
			_commandTarget.SendMessage(&msg);
		} else if (strncasecmp(buf, "priv:", 5) == 0) {
			BMessage msg(_privMessage);
			msg.AddString("users", &buf[5]);
			_commandTarget.SendMessage(&msg);
		} else if (strncasecmp(buf, "audio://", 8) == 0) {
			BMessage msg(B_REFS_RECEIVED);
			BString temp("http://");
			temp += &buf[8];
			msg.AddString("be:url", temp);
			be_roster->Launch("audio/x-scpls", &msg);
		} else be_roster->Launch(strncasecmp(buf, "mailto:", 7) ? "text/html" : "text/x-email", 1, const_cast<char**>(&buf));

		delete [] allocBuf;
	}
	BTextView::MouseDown(where);
}

void ReflowingTextView::FixTextRect()
{
	BRect t(Frame());
	SetTextRect(t);
}

void ReflowingTextView::Clear()
{
	Delete(0, TextLength() - 1);
	_urls.Clear();
}

void ReflowingTextView::AddURLRegion(uint32 start, uint32 len, const BString& optHiddenURL)
{
	_urls.AddTail(URLLink(start, len, optHiddenURL));
}

void ReflowingTextView::SetCommandURLTarget(const BMessenger& target, const BMessage& queryMsg, const BMessage& privMsg)
{
	_commandTarget = target;
	_queryMessage  = queryMsg;
	_privMessage   = privMsg;
}

const ReflowingTextView::URLLink* ReflowingTextView::GetURLAt(BPoint pt) const
{
	uint32 offset = (uint32) OffsetAt(pt);
	for (int i = _urls.GetNumItems() - 1; i >= 0; i--) {
		const URLLink& url = _urls[i];
		if ((offset >= url.GetStart()) && (offset < url.GetStart() + url.GetLength())) return &url;
	}
	return NULL;
}
