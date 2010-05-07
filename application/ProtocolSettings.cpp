/*
 * Copyright 2009-2010, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2003-2009, IM Kit Team. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Michael Davidson, slaad@bong.com.au
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>

#include <CheckBox.h>
#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <GroupLayout.h>
#include <GroupLayoutBuilder.h>
#include <Menu.h>
#include <MenuItem.h>
#include <MenuField.h>
#include <Message.h>
#include <Path.h>
#include <PopUpMenu.h>
#include <Resources.h>
#include <ScrollView.h>
#include <StringView.h>
#include <TextControl.h>
#include <TextView.h>
#include <TypeConstants.h>

#include <libinterface/NotifyingTextView.h>

#include "CayaProtocol.h"
#include "CayaResources.h"
#include "ProtocolManager.h"
#include "ProtocolSettings.h"

#define _T(str) (str)

const float kDividerWidth = 1.0f;


ProtocolSettings::ProtocolSettings(CayaProtocol* cayap)
	: fProtocol(cayap),
	fTemplate(new BMessage()),
	fSettings(new BMessage())
{
	_Init();
}


ProtocolSettings::~ProtocolSettings()
{
	delete fTemplate;
	delete fSettings;
}


status_t
ProtocolSettings::InitCheck() const
{
	return fStatus;
}


BList*
ProtocolSettings::Accounts() const
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return NULL;

	path.Append("Caya/Protocols");
	path.Append(fProtocol->GetSignature());
	if (create_directory(path.Path(), 0755) != B_OK)
		return NULL;

	BList* list = new BList();
	BDirectory dir(path.Path());
	BEntry entry;
	while (dir.GetNextEntry(&entry) == B_OK) {
		BFile file(&entry, B_READ_ONLY);
		BMessage msg;

		if (msg.Unflatten(&file) == B_OK) {
			char buffer[B_PATH_NAME_LENGTH];
			if (entry.GetName(buffer) == B_OK) {
				BString* string = new BString(buffer);
				list->AddItem(string);
			}
		}
	}

	return list;
}


status_t
ProtocolSettings::Load(const char* account)
{
	status_t ret = B_ERROR;

	// Find user's settings path
	BPath path;
	if ((ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path)) != B_OK)
		return ret;

	// Create path
	path.Append("Caya/Protocols");
	path.Append(fProtocol->GetSignature());
	if ((ret = create_directory(path.Path(), 0755)) != B_OK)
		return ret;

	// Load settings file
	path.Append(account);
	BFile file(path.Path(), B_READ_ONLY);
	return fSettings->Unflatten(&file);
}


status_t
ProtocolSettings::Save(const char* account)
{
	status_t ret = B_ERROR;

	// Find user's settings path
	BPath path;
	if ((ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path)) != B_OK)
		return ret;

	// Create path
	path.Append("Caya/Protocols");
	path.Append(fProtocol->GetSignature());
	if ((ret = create_directory(path.Path(), 0755)) != B_OK)
		return ret;

	// Load settings file
	path.Append(account);
	BFile file(path.Path(), B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	return fSettings->Flatten(&file);
}


status_t
ProtocolSettings::Delete(const char* account)
{
	status_t ret = B_ERROR;

	// Find user's settings path
	BPath path;
	if ((ret = find_directory(B_USER_SETTINGS_DIRECTORY, &path)) != B_OK)
		return ret;

	// Create path
	path.Append("Caya/Protocols");
	path.Append(fProtocol->GetSignature());
	if ((ret = create_directory(path.Path(), 0755)) != B_OK)
		return ret;
	path.Append(account);

	// Delete settings file
	BEntry entry(path.Path());
	if ((ret = entry.Remove()) != B_OK)
		return ret;

	delete fSettings;
	fSettings = new BMessage();
	return B_OK;
}


void
ProtocolSettings::BuildGUI(BView* parent)
{
	if (!parent)
		debugger("Couldn't build protocol's settings GUI on a NULL parent!");

	BMessage curr;
	float inset = ceilf(be_plain_font->Size() * 0.7f);

	// Setup layout
	parent->SetLayout(new BGroupLayout(B_VERTICAL));
	BGroupLayoutBuilder layout(B_VERTICAL, inset);

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
					for (int j = 0; curr.FindString("valid_value", j); j++)  {
						BMenuItem* item
							= new BMenuItem(curr.FindString("valid_value", j), NULL);
						menu->AddItem(item);
					}

					value = fSettings->FindString(name);
					if (value)
						menu->FindItem(value)->SetMarked(true);
				} else {
					// It's a free-text setting
					if (curr.FindBool("multi_line", &multiLine) != B_OK)
						multiLine = false;

					value = fSettings->FindString(name);
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
					status_t hasValue = fSettings->FindInt32(name, 0, &def);

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

					if (fSettings->FindInt32(name, &v) == B_OK) {
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

				if (fSettings->FindBool(name, &active) != B_OK) {
					if (curr.FindBool("default", &active) != B_OK)
						active = false;
				}

				control = new BCheckBox(name, _T(desc), NULL);
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
					control = new BTextControl(name, _T(desc), value, NULL);
					if (secret) {
						dynamic_cast<BTextControl*>(control)->TextView()->HideTyping(true);
						dynamic_cast<BTextControl*>(control)->SetText(_T(value));
					}
					dynamic_cast<BTextControl*>(control)->SetDivider(kDividerWidth);
				} else {
					BStringView* label = new BStringView("NA", _T(desc), B_WILL_DRAW);
					layout.Add(label);

					NotifyingTextView* textView
						= new NotifyingTextView(name);
					control = new BScrollView("NA", textView, 0, false, true);
					textView->SetText(_T(value));			
				}
			} else {
				control = new BMenuField(name, _T(desc), menu);
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
}


status_t
ProtocolSettings::SaveGUI(BView* parent)
{
	if (!parent)
		debugger("Couldn't save protocol's settings GUI on a NULL parent!");

	BMessage cur;
	for (int32 i = 0; fTemplate->FindMessage("setting", i, &cur) == B_OK; i++) {
		const char* name = cur.FindString("name");

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
		if (textControl) {
			switch (type) {
				case B_STRING_TYPE:
					fSettings->AddString(name, textControl->Text());
					break;
				case B_INT32_TYPE:
					fSettings->AddInt32(name, atoi(textControl->Text()));
					break;
				default:
					return B_ERROR;
			}
		}

		BMenuField* menuField
			= dynamic_cast<BMenuField*>(view);
		if (menuField) {
			BMenuItem* item = menuField->Menu()->FindMarked();
			if (!item)
				return B_ERROR;

			switch (type) {
				case B_STRING_TYPE:
					fSettings->AddString(name, item->Label());
					break;
				case B_INT32_TYPE:
					fSettings->AddInt32(name, atoi(item->Label()));
					break;
				default:
					return B_ERROR;
			}
		}

		BCheckBox* checkBox
			= dynamic_cast<BCheckBox*>(view);
		if (checkBox)
			fSettings->AddBool(name, (checkBox->Value() == B_CONTROL_ON));

		NotifyingTextView* textView
			= dynamic_cast<NotifyingTextView*>(view);
		if (textView)
			fSettings->AddString(name, textView->Text());
	}

fSettings->PrintToStream();
	return B_OK;
}


void
ProtocolSettings::_Init()
{
	// Find protocol add-on
	BPath* dllPath = ProtocolManager::Get()->GetProtocolPath(
		fProtocol->GetSignature());
	BFile file(dllPath->Path(), B_READ_ONLY);
	if (file.InitCheck() < B_OK) {
		fStatus = file.InitCheck();
		return;
	}

	BResources resources(&file);
	if (resources.InitCheck() != B_OK) {
		fStatus = resources.InitCheck();
		return;
	}

	size_t size;
	const void* data = resources.LoadResource(B_MESSAGE_TYPE,
		kProtocolSettingsTemplate, &size);
	if (!data) {
		fStatus = B_BAD_VALUE;
		return;
	}

	// Load protocol's settings template
	fTemplate->Unflatten((const char*)data);
}
