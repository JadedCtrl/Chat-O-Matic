 /*
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 *		Jaidyn Levesque, jadedctrl@teknik.io
 */

#include "ProtocolTemplate.h"

#include <cstdio>

#include <CheckBox.h>
#include <GroupLayoutBuilder.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <PopUpMenu.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>

#include <libinterface/NotifyingTextView.h>

#include "CayaProtocol.h"
#include "CayaProtocolAddOn.h"


const float kDividerWidth = 1.0f;


ProtocolTemplate::ProtocolTemplate(CayaProtocol* protocol, const char* type)
	:
	fProtocol(protocol),
	fTemplate(new BMessage())
{
	// Load protocol's settings template
	BMessage settingsTemplate = fProtocol->SettingsTemplate(type);
	*fTemplate = settingsTemplate;
}


ProtocolTemplate::~ProtocolTemplate()
{
	delete fTemplate;
}


CayaProtocol*
ProtocolTemplate::Protocol() const
{
	return fProtocol;
}


status_t
ProtocolTemplate::Load(BView* parent, BMessage* settings)
{
	if (!parent)
		debugger("Couldn't build protocol's settings GUI on a NULL parent!");
	BMessage curr;

	// Setup layout
	parent->SetLayout(new BGroupLayout(B_VERTICAL));
	BGroupLayoutBuilder layout(B_VERTICAL);

	for (int32 i = 0; fTemplate->FindMessage("setting", i, &curr) == B_OK; i++) {
		char temp[512];

		// Get stuff from settings template
		const char* name = curr.FindString("name");
		const char* desc = curr.FindString("description");
		const char* value = NULL;
		int32 type = -1;
		bool secret = false;
		bool freeText = true;
		bool multiLine = false;
		BView* control = NULL;
		BMenu* menu = NULL;

		// Ignore settings with errors
		if (curr.FindInt32("type", &type) != B_OK)
			continue;

		switch (type) {
			case B_STRING_TYPE: {
				if (curr.FindString("valid_value")) {
					// It's a "select one of these" setting
					freeText = false;

					menu = new BPopUpMenu(name);
					for (int j = 0; curr.FindString("valid_value", j); j++) {
						BMenuItem* item
							= new BMenuItem(curr.FindString("valid_value", j),
								NULL);
						menu->AddItem(item);
					}

					if (settings)
						value = settings->FindString(name);
					if (value)
						menu->FindItem(value)->SetMarked(true);
				} else {
					// It's a free-text setting
					if (curr.FindBool("multi_line", &multiLine) != B_OK)
						multiLine = false;

					if (settings)
						value = settings->FindString(name);
					if (!value)
						value = curr.FindString("default");

					if (curr.FindBool("is_secret",&secret) != B_OK)
						secret = false;
				}
				break;
			}
			case B_INT32_TYPE: {
				if (curr.FindInt32("valid_value")) {
					// It's a "select one of these" setting
					freeText = false;

					menu = new BPopUpMenu(name);

					int32 def = 0;
					status_t hasValue = B_ERROR;

					if (settings)
						settings->FindInt32(name, 0, &def);

					if (hasValue != B_OK)
						hasValue = curr.FindInt32("default", 0, &def);

					int32 v = 0;
					for (int32 j = 0; curr.FindInt32("valid_value", j, &v) == B_OK; j++) {
						sprintf(temp, "%ld", v);

						BMenuItem* item = new BMenuItem(temp, NULL);

						if (hasValue != B_OK && j == 0)
							item->SetMarked(true);
						else if ((hasValue == B_OK) && (def == v))
							item->SetMarked(true);

						menu->AddItem(item);
					}
				} else {
					// It's a free-text (but number) setting
					int32 v = 0;

					if (settings && settings->FindInt32(name, &v) == B_OK) {
						sprintf(temp,"%ld", v);
						value = temp;
					} else if (curr.FindInt32("default", &v) == B_OK) {
						sprintf(temp,"%ld", v);
						value = temp;
					}

					if (curr.FindBool("is_secret",&secret) != B_OK)
						secret = false;
				}
				break;
			}
			case B_BOOL_TYPE: {
				bool active;

				if (settings && settings->FindBool(name, &active) != B_OK) {
					if (curr.FindBool("default", &active) != B_OK)
						active = false;
				}

				control = new BCheckBox(name, desc, NULL);
				if (active)
					dynamic_cast<BCheckBox*>(control)->SetValue(B_CONTROL_ON);
				break;
			}
			default:
				continue;
		}

		if (!value)
			value = "";

		if (!control) {
			if (freeText) {
				if (!multiLine) {
					control = new BTextControl(name, desc, value, NULL);
					if (secret) {
						dynamic_cast<BTextControl*>(control)->TextView()->HideTyping(true);
						dynamic_cast<BTextControl*>(control)->SetText(value);
					}
					dynamic_cast<BTextControl*>(control)->SetDivider(
						kDividerWidth);
				} else {
					BStringView* label = new BStringView("NA", desc,
						B_WILL_DRAW);
					layout.Add(label);

					NotifyingTextView* textView
						= new NotifyingTextView(name);
					control = new BScrollView("NA", textView, 0, false, true);
					textView->SetText(value);			
				}
			} else {
				control = new BMenuField(name, desc, menu);
				dynamic_cast<BMenuField*>(control)->SetDivider(kDividerWidth);
			}
		}

#if 0
		if (curr.FindString("help"))
			gHelper.SetHelp(control, strdup(curr.FindString("help")));
#endif

		layout.Add(control);
	}

	layout.AddGlue();
	parent->AddChild(layout);

	return B_OK;
}


status_t
ProtocolTemplate::Save(BView* parent, BMessage* settings, BString* errorText)
{
	if (!parent)
		debugger("Couldn't save protocol's settings GUI on a NULL parent!");

	BMessage cur;
	for (int32 i = 0; fTemplate->FindMessage("setting", i, &cur) == B_OK; i++) {
		const char* name = cur.FindString("name");
		BString error = cur.FindString("error");

		// Skip NULL names
		if (!name)
			continue;

		int32 type = -1;
		if (cur.FindInt32("type", &type) != B_OK)
			continue;

		BView* view = parent->FindView(name);
		if (!view)
			continue;

		BTextControl* textControl
			= dynamic_cast<BTextControl*>(view);

		if (textControl && BString(textControl->Text()).IsEmpty() == true
			&& error.IsEmpty() == false)
		{
			if (errorText != NULL)
				errorText->SetTo(error);
			return B_BAD_VALUE;
		}
		else if (textControl)
			switch (type) {
				case B_STRING_TYPE:
					settings->AddString(name, textControl->Text());
					break;
				case B_INT32_TYPE:
					settings->AddInt32(name, atoi(textControl->Text()));
					break;
				default:
					return B_BAD_TYPE;
			}

		BMenuField* menuField
			= dynamic_cast<BMenuField*>(view);
		if (menuField) {
			BMenuItem* item = menuField->Menu()->FindMarked();
			if (!item)
				return B_ERROR;

			switch (type) {
				case B_STRING_TYPE:
					settings->AddString(name, item->Label());
					break;
				case B_INT32_TYPE:
					settings->AddInt32(name, atoi(item->Label()));
					break;
				default:
					return B_BAD_TYPE;
			}
		}

		BCheckBox* checkBox
			= dynamic_cast<BCheckBox*>(view);
		if (checkBox)
			settings->AddBool(name, (checkBox->Value() == B_CONTROL_ON));

		NotifyingTextView* textView
			= dynamic_cast<NotifyingTextView*>(view);
		if (textView)
			settings->AddString(name, textView->Text());
	}

	return B_OK;
}
