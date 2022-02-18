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

#include "ProtocolSettings.h"

#include <Directory.h>
#include <File.h>
#include <Message.h>
#include <ObjectList.h>
#include <Path.h>

#include "ChatProtocolAddOn.h"
#include "Utils.h"


ProtocolSettings::ProtocolSettings(ChatProtocolAddOn* addOn)
	:
	fTemplate(addOn->Protocol(), "account"),
	fAddOn(addOn)
{
}


status_t
ProtocolSettings::InitCheck() const
{
	return fStatus;
}


ChatProtocolAddOn*
ProtocolSettings::AddOn() const
{
	return fAddOn;
}


BObjectList<BString>
ProtocolSettings::Accounts() const
{
	BObjectList<BString> list(true);

	BPath path = AccountPath(fAddOn->Signature(), fAddOn->ProtoSignature());

	if (path.InitCheck() != B_OK)
		return list;

	BDirectory dir(path.Path());
	BEntry entry;
	while (dir.GetNextEntry(&entry) == B_OK) {
		BFile file(&entry, B_READ_ONLY);
		BMessage msg;

		if (msg.Unflatten(&file) == B_OK) {
			char buffer[B_PATH_NAME_LENGTH];
			if (entry.GetName(buffer) == B_OK)
				list.AddItem(new BString(buffer));
		}
	}
	return list;
}


status_t
ProtocolSettings::Load(const char* account, BView* parent)
{
	BMessage* settings = NULL;

	if (account) {
		status_t ret = Load(account, &settings);
		if (ret != B_OK)
			return ret;
	}
	return fTemplate.Load(parent, settings);
}


status_t
ProtocolSettings::Load(const char* account, BMessage** settings)
{
	if (!account || !settings)
		return B_BAD_VALUE;

	status_t ret = B_ERROR;

	// Find user's settings path
	BPath path = AccountPath(fAddOn->Signature(), fAddOn->ProtoSignature());

	if ((ret = path.InitCheck()) != B_OK)
		return ret;

	// Load settings file
	path.Append(account);
	BFile file(path.Path(), B_READ_ONLY);
	BMessage* msg = new BMessage();
	ret = msg->Unflatten(&file);
	*settings = msg;
	return ret;
}


status_t
ProtocolSettings::Save(const char* account, BView* parent, BString* errorText)
{
	if (!parent)
		debugger("Couldn't save protocol's settings GUI on a NULL parent!");

	BMessage settings;
	status_t status = fTemplate.Save(parent, &settings, errorText);

	if (status != B_OK)
		return status;
	return Save(account, settings);
}


status_t
ProtocolSettings::Save(const char* account, BMessage settings)
{
	// Find user's settings path
	BPath path = AccountPath(fAddOn->Signature(), fAddOn->ProtoSignature());

	status_t ret;
	if ((ret = path.InitCheck()) != B_OK)
		return ret;

	// Load settings file
	path.Append(account);
	BFile file(path.Path(), B_CREATE_FILE | B_ERASE_FILE | B_WRITE_ONLY);
	return settings.Flatten(&file);
}


status_t
ProtocolSettings::Rename(const char* from, const char* to)
{
	status_t ret = B_ERROR;

	// Find user's settings path
	BPath path = AccountPath(fAddOn->Signature(), fAddOn->ProtoSignature());

	if ((ret = path.InitCheck()) != B_OK)
		return ret;

	path.Append(from);

	// Delete settings file
	BEntry entry(path.Path());
	if ((ret = entry.Rename(to)) != B_OK)
		return ret;

	return B_OK;
}


status_t
ProtocolSettings::Delete(const char* account)
{
	status_t ret = B_ERROR;

	// Find user's settings path
	BPath path = AccountPath(fAddOn->Signature(), fAddOn->ProtoSignature());

	if ((ret = path.InitCheck()) != B_OK)
		return ret;

	path.Append(account);

	// Delete settings file
	BEntry entry(path.Path());
	if ((ret = entry.Remove()) != B_OK)
		return ret;

	return B_OK;
}


