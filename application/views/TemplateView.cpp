/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include "TemplateView.h"

#include <Button.h>
#include <ControlLook.h>
#include <CheckBox.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <PopUpMenu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <ScrollView.h>
#include <TextControl.h>
#include <Window.h>


TemplateView::TemplateView(const char* name)
	: BView(name, B_WILL_DRAW)
{
}


void
TemplateView::AttachedToWindow()
{
	// Once we are attached to window, the GUI is already created
	// so we can set our window as target for messages
	for (int32 i = 0; i < CountChildren(); i++) {
		BView* child = ChildAt(i);

		BMenu* menu = dynamic_cast<BMenu*>(child);
		BMenuField* menuField
			= dynamic_cast<BMenuField*>(child);
		BTextControl* textControl
			= dynamic_cast<BTextControl*>(child);
		BTextView* textView = dynamic_cast<BTextView*>(child);
		BCheckBox* checkBox = dynamic_cast<BCheckBox*>(child);

		if (menuField)
			menu = menuField->Menu();

		if (menu) {
			if (i == 0)
				menu->MakeFocus(true);
			menu->SetTargetForItems(Window());
		}

		if (textControl)
			if (i == 0)
				textControl->MakeFocus(true);

		if (checkBox)
			if (i == 0)
				checkBox->MakeFocus(true);

		if (textView)
			if (i == 0)
				textView->MakeFocus(true);
	}
}
