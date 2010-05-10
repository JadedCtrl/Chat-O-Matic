/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <new>

#include <ControlLook.h>
#include <LayoutUtils.h>
#include <Looper.h>
#include <MenuItem.h>
#include <PopUpMenu.h>
#include <Window.h>

#include <binary_compatibility/Interface.h>

#include <libinterface/BitmapUtils.h>

#include "ToolButton.h"

const float kPopUpMarkerSize   = 5.0f;
const float kPopUpMarkerTop    = 0.5f;
const float kPopUpMarkerRect   = (kPopUpMarkerSize * 2) + 2;
const uint32 kToolbarIconSize  = 16; // this should go on BControlLook


ToolButton::ToolButton(const char* name, const char* label, BMessage* message,
					   uint32 flags)
	: BControl(name, label, message,
	           flags | B_WILL_DRAW | B_FULL_UPDATE_ON_RESIZE),
	fBitmap(NULL),
	fMenu(NULL),
	fPreferredSize(-1, -1)
{
	SetFontSize(be_plain_font->Size() * 0.85);
}


ToolButton::ToolButton(const char* label, BMessage* message)
	: BControl(NULL, label, message,
			   B_WILL_DRAW | B_NAVIGABLE | B_FULL_UPDATE_ON_RESIZE),
	fBitmap(NULL),
	fMenu(NULL),
	fPreferredSize(-1, -1)
{
	SetFontSize(be_plain_font->Size() * 0.85);
}


ToolButton::ToolButton(BMessage* archive)
	: BControl(archive),
	fBitmap(NULL),
	fMenu(NULL),
	fPreferredSize(-1, -1)
{
	SetFontSize(be_plain_font->Size() * 0.85);
}


ToolButton::~ToolButton()
{
}


BArchivable*
ToolButton::Instantiate(BMessage* archive)
{
	if (validate_instantiation(archive, "ToolButton"))
		return new(std::nothrow) ToolButton(archive);
	return NULL;
}


status_t
ToolButton::Archive(BMessage* archive, bool deep) const
{
	status_t err = BControl::Archive(archive, deep);
	if (err != B_OK)
		return err;

	return err;
}


BSize
ToolButton::MinSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMinSize(),
		_ValidatePreferredSize());
}


BSize
ToolButton::MaxSize()
{
	return BLayoutUtils::ComposeSize(ExplicitMaxSize(),
		_ValidatePreferredSize());
}


BSize
ToolButton::PreferredSize()
{
	return BLayoutUtils::ComposeSize(ExplicitPreferredSize(),
		_ValidatePreferredSize());
}


void
ToolButton::Draw(BRect updateRect)
{
	BRect bounds(Bounds());
	rgb_color base = ui_color(B_PANEL_BACKGROUND_COLOR);
	uint32 flags = be_control_look->Flags(this);

	// Draw background and borders
	rgb_color button = IsEnabled() ? tint_color(base, 1.07) : base;
	be_control_look->DrawButtonBackground(this, bounds, updateRect,
		button, flags, 0);
	be_control_look->DrawBorder(this, bounds, updateRect,
		base, B_PLAIN_BORDER, flags);

	// Calculate popup marker space
	if (fMenu)
		bounds.right -= kPopUpMarkerRect;

	// Draw bitmap
	if (Bitmap()) {
		SetDrawingMode(B_OP_ALPHA);
		SetBlendingMode(B_PIXEL_ALPHA, B_ALPHA_OVERLAY);

		BRect frame(bounds);
		BPoint center = _Center(frame);
		frame.bottom = frame.top + kToolbarIconSize + 4;
		frame.top += 2;
		frame.left = center.x - (kToolbarIconSize / 2);
		frame.right = center.x + (kToolbarIconSize / 2);

		DrawBitmap(fBitmap, fBitmap->Bounds(), frame, B_FILTER_BITMAP_BILINEAR);
		bounds.top = frame.bottom;
	}

	// Draw label
	if (Label())
		be_control_look->DrawLabel(this, Label(), bounds, updateRect,
			base, flags, BAlignment(B_ALIGN_CENTER, B_ALIGN_MIDDLE));

	if (fMenu) {
		// Draw popup marker
		bounds = Bounds();
		bounds.left = bounds.right - kPopUpMarkerRect;
		bounds.InsetBy(1, 1);

		be_control_look->DrawButtonBackground(this, bounds, updateRect,
			tint_color(button, B_DARKEN_2_TINT), flags, 0);

		float tint = IsEnabled() ? B_DARKEN_4_TINT : B_DARKEN_1_TINT;
		BPoint center(_Center(bounds));
		BPoint triangle[3];
		triangle[0] = center + BPoint(-(kPopUpMarkerSize / 2.0), -kPopUpMarkerTop);
		triangle[1] = center + BPoint(kPopUpMarkerSize, -kPopUpMarkerTop);
		triangle[2] = center + BPoint(0.0, (kPopUpMarkerSize / 2.0f) - kPopUpMarkerTop);

		uint32 origFlags = Flags();
		SetFlags(origFlags | B_SUBPIXEL_PRECISE);
		SetHighColor(tint_color(base, tint));
		FillTriangle(triangle[0], triangle[1], triangle[2]);
		SetFlags(origFlags);
	}
}


void
ToolButton::AttachedToWindow()
{
	BControl::AttachedToWindow();
	SetViewColor(B_TRANSPARENT_COLOR);
}


void
ToolButton::MouseDown(BPoint where)
{
	if (!IsEnabled())
		return;

	BRect rect(Bounds());
	rect.left = rect.right - kPopUpMarkerRect;

	if (fMenu && rect.Contains(where)) {
		BPoint coords(rect.left + 2, rect.bottom + 1);
		ConvertToScreen(&coords);

		BMenuItem* selected = fMenu->Go(coords, false, true);
		if (selected) {
			BLooper* looper;
			BHandler* target = selected->Target(&looper);
			(void)looper->PostMessage(selected->Message(), target);
		}
	} else {
		SetValue(B_CONTROL_ON);

		if (Window()->Flags() & B_ASYNCHRONOUS_CONTROLS) {
			SetTracking(true);
			SetMouseEventMask(B_POINTER_EVENTS, B_LOCK_WINDOW_FOCUS);
		} else {
			BRect bounds = Bounds();
			uint32 buttons;

			do {
				Window()->UpdateIfNeeded();
				snooze(40000);

				GetMouse(&where, &buttons, true);

				bool inside = bounds.Contains(where);

				if ((Value() == B_CONTROL_ON) != inside)
					SetValue(inside ? B_CONTROL_ON : B_CONTROL_OFF);
			} while (buttons != 0);

			if (Value() == B_CONTROL_ON)
				Invoke();
		}
	}
}


void
ToolButton::MouseMoved(BPoint where, uint32 transit, const BMessage* message)
{
	if (!IsTracking())
		return;

	bool inside = Bounds().Contains(where);

	if ((Value() == B_CONTROL_ON) != inside)
		SetValue(inside ? B_CONTROL_ON : B_CONTROL_OFF);
}


void
ToolButton::MouseUp(BPoint where)
{
	if (!IsTracking())
		return;

	if (Bounds().Contains(where))
		Invoke();

	SetTracking(false);
}


void
ToolButton::SetLabel(const char* string)
{
	BControl::SetLabel(string);
}


void
ToolButton::MessageReceived(BMessage* message)
{
	BControl::MessageReceived(message);
}


void
ToolButton::WindowActivated(bool active)
{
	BControl::WindowActivated(active);
}


status_t
ToolButton::Invoke(BMessage* message)
{
	Sync();
	snooze(50000);

	status_t err = BControl::Invoke(message);

	SetValue(B_CONTROL_OFF);

	return err;
}


status_t
ToolButton::Perform(perform_code code, void* data)
{
	switch (code) {
		case PERFORM_CODE_MIN_SIZE:
			((perform_data_min_size*)data)->return_value
					= ToolButton::MinSize();
			return B_OK;
		case PERFORM_CODE_MAX_SIZE:
			((perform_data_max_size*)data)->return_value
					= ToolButton::MaxSize();
			return B_OK;
		case PERFORM_CODE_PREFERRED_SIZE:
			((perform_data_preferred_size*)data)->return_value
					= ToolButton::PreferredSize();
			return B_OK;
		case PERFORM_CODE_LAYOUT_ALIGNMENT:
			((perform_data_layout_alignment*)data)->return_value
					= ToolButton::LayoutAlignment();
			return B_OK;
		case PERFORM_CODE_HAS_HEIGHT_FOR_WIDTH:
			((perform_data_has_height_for_width*)data)->return_value
					= ToolButton::HasHeightForWidth();
			return B_OK;
		case PERFORM_CODE_GET_HEIGHT_FOR_WIDTH: {
			perform_data_get_height_for_width* _data
					= (perform_data_get_height_for_width*)data;
			ToolButton::GetHeightForWidth(_data->width, &_data->min, &_data->max,
					&_data->preferred);
			return B_OK;
		}
		case PERFORM_CODE_INVALIDATE_LAYOUT: {
			perform_data_invalidate_layout* _data
					= (perform_data_invalidate_layout*)_data;
			ToolButton::InvalidateLayout(_data->descendants);
			return B_OK;
		}
		case PERFORM_CODE_DO_LAYOUT:
			ToolButton::DoLayout();
			return B_OK;
	}

	return BControl::Perform(code, data);
}


void
ToolButton::InvalidateLayout(bool descendants)
{
	fPreferredSize.Set(-1, -1);
	BControl::InvalidateLayout(descendants);
}


BBitmap*
ToolButton::Bitmap() const
{
	return fBitmap;
}


void
ToolButton::SetBitmap(BBitmap* bitmap)
{
	fBitmap = bitmap;
}


BPopUpMenu*
ToolButton::Menu() const
{
	return fMenu;
}


void
ToolButton::SetMenu(BPopUpMenu* menu)
{
	fMenu = menu;
}


BPoint
ToolButton::_Center(BRect rect)
{
	BPoint center(roundf((rect.left + rect.right) / 2.0),
		roundf((rect.top + rect.bottom) / 2.0));
	return center;
}


BSize
ToolButton::_ValidatePreferredSize()
{
	if (fPreferredSize.width < 0) {
		float startWidth = 10;
		float minSize = 25;
		float minWidth = minSize;

		if (fMenu) {
			startWidth += kPopUpMarkerRect;
			minWidth += kPopUpMarkerRect;
		}

		// Width
		float width = startWidth + (float)ceil(StringWidth(Label()));
		if (width < minWidth)
			width = minWidth;

		fPreferredSize.width = width;

		// Height
		fPreferredSize.height = 0;

		if (Label()) {
			font_height fontHeight;
			GetFontHeight(&fontHeight);

			fPreferredSize.height
				+= ceilf((fontHeight.ascent + fontHeight.descent) * 1.5);
		}

		if (Bitmap())
			fPreferredSize.height += kToolbarIconSize + 6;

		if (Bitmap() && Label())
			fPreferredSize.height += 4;

		if (fPreferredSize.height < minSize)
			fPreferredSize.height = minSize;

		ResetLayoutInvalidation();
	}

	return fPreferredSize;
}
