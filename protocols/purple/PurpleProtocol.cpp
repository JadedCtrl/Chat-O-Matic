/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include "PurpleProtocol.h"

#include <iostream>

#include <Application.h>
#include <Catalog.h>
#include <Resources.h>
#include <Roster.h>
#include <TranslationUtils.h>

#include <ChatProtocolMessages.h>

#include "Purple.h"
#include "PurpleMessages.h"


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleProtocol"


BMessenger* kAppMessenger = NULL;


ChatProtocol*
protocol_at(int32 i)
{
	BMessenger* msgr = ensure_app_messenger();
	if (msgr == NULL)
		return NULL;

	BMessage* msg = new BMessage(PURPLE_REQUEST_PROTOCOL_INFO);
	msg->AddInt64("thread_id", find_thread(NULL));
	msg->AddInt32("index", i);
	msgr->SendMessage(msg);

	BMessage protoInfo = receive_message();
	BString name = protoInfo.FindString("name");
	BString id = protoInfo.FindString("id");

	return (ChatProtocol*)new PurpleProtocol(name, id, protoInfo);
}


int32
protocol_count()
{
	BMessenger* msgr = ensure_app_messenger();
	if (msgr == NULL)
		return 0;

	BMessage* msg = new BMessage(PURPLE_REQUEST_PROTOCOL_COUNT);
	msg->AddInt64("thread_id", find_thread(NULL));
	msgr->SendMessage(msg);

	thread_id sender;
	return receive_data(&sender, NULL, 0);
}


const char*
signature()
{
	return PURPLE_ADDON;
}


const char*
friendly_signature()
{
	return "Purple";
}


uint32
version()
{
	return APP_VERSION_1_ALPHA_1;
}


BMessenger*
ensure_app_messenger()
{
	if (kAppMessenger == NULL || kAppMessenger->IsValid() == false) {
		if (ensure_app() == true)
			kAppMessenger = new BMessenger(PURPLE_SIGNATURE);
	}
	return kAppMessenger;
}


bool
ensure_app()
{
	BRoster roster;
	if (roster.IsRunning(PURPLE_SIGNATURE) == true)
		return true;

	if (roster.Launch(PURPLE_SIGNATURE) == B_OK) {
		snooze(100000);
		return true;
	}

	app_info aInfo;
	be_app->GetAppInfo(&aInfo);
	BPath protoPath(&aInfo.ref);
	protoPath.GetParent(&protoPath);
	protoPath.Append(BString(APP_NAME).ToLower().Append("/purple"));

	entry_ref protoRef;
	BEntry(protoPath.Path()).GetRef(&protoRef);
	if (roster.Launch(&protoRef) == B_OK) {
		snooze(100000);
		return true;
	}

	std::cerr << "libpurple add-on could not find its binary! All is lost!\n";
	return false;
}


status_t
connect_thread(void* data)
{
	PurpleProtocol* protocol = (PurpleProtocol*)data;
	while (true) {
		BMessage* msg = new BMessage(receive_message());
		switch (msg->what) {
			case PURPLE_SHUTDOWN_ADDON: {
				BMessage* disabled = new BMessage(IM_MESSAGE);
				disabled->AddInt32("im_what", IM_PROTOCOL_DISABLE);
				protocol->SendMessage(disabled);
				break;
			}
			case PURPLE_REGISTER_COMMANDS:
				protocol->Process(msg);
				break;
			default:
				protocol->SendMessage(msg);
		}
	}
}


BMessage
receive_message()
{
	thread_id sender;
	int32 size = receive_data(&sender, NULL, 0);
	char buffer[size];
	receive_data(&sender, buffer, size);
	BMessage temp;
	temp.Unflatten(buffer);
	return temp;
}


PurpleProtocol::PurpleProtocol(BString name, BString id, BMessage settings)
	:
	fSignature(id),
	fFriendlySignature(name),
	fIcon(NULL)
{
	fIconName = settings.GetString("icon", "");
	settings.FindMessage("templates", &fTemplates);

	BPath path;
	_FindIcon(&path, B_SYSTEM_DATA_DIRECTORY);
	_FindIcon(&path, B_SYSTEM_NONPACKAGED_DATA_DIRECTORY);
	_FindIcon(&path, B_USER_DATA_DIRECTORY);
	_FindIcon(&path, B_USER_NONPACKAGED_DATA_DIRECTORY);

	if (path.InitCheck() == B_OK) {
		BFile iconFile(path.Path(), B_READ_ONLY);
		fIcon = BTranslationUtils::GetBitmap(&iconFile);
	}
}


PurpleProtocol::~PurpleProtocol()
{
	Shutdown();
}


status_t
PurpleProtocol::Init(ChatProtocolMessengerInterface* interface)
{
	fMessenger = interface;
	return B_OK;
}


status_t
PurpleProtocol::Shutdown()
{
	BMessage* disconnect = new BMessage(PURPLE_DISCONNECT_ACCOUNT);
	_SendPrplMessage(disconnect);

	kill_thread(fBirdThread);
	return B_OK;
}

status_t
PurpleProtocol::Process(BMessage* msg)
{
	switch(msg->what)
	{
		case IM_MESSAGE:
			_SendPrplMessage(msg);
			return B_OK;
		case PURPLE_REGISTER_COMMANDS:
		{
			BMessage cmd;
			for (int i = 0; msg->FindMessage("command", i, &cmd) == B_OK; i++)
				fCommands.AddItem(new BMessage(cmd));

			BMessage* reload = new BMessage(IM_MESSAGE);
			reload->AddInt32("im_what", IM_PROTOCOL_RELOAD_COMMANDS);
			SendMessage(reload);
			break;
		}
	}
	return B_ERROR;
}


status_t
PurpleProtocol::UpdateSettings(BMessage* msg)
{
	ensure_app();
	fPrplMessenger = new BMessenger(PURPLE_SIGNATURE);
	msg->what = PURPLE_CONNECT_ACCOUNT;
	_SendPrplMessage(msg);

	fBirdThread = spawn_thread(connect_thread, "god_has_abandoned_us",
		B_NORMAL_PRIORITY, (void*)this);

	if (fBirdThread < B_OK)
		return B_ERROR;

	BMessage* account = new BMessage(PURPLE_REGISTER_THREAD);
	account->AddInt64("thread_id", fBirdThread);
	account->AddString("addon_cache", fAddOnCachePath.Path());
	account->AddString("account_cache", fAccountCachePath.Path());
	_SendPrplMessage(account);

	resume_thread(fBirdThread);
	return B_OK;
}


BMessage
PurpleProtocol::SettingsTemplate(const char* name)
{
	BMessage temp;

	if (strcmp(name, "roster") == 0)
		return _RosterTemplate();
	else if (strcmp(name, "account") == 0)
		fTemplates.FindMessage("account", &temp);
	else if (strcmp(name, "create_room") == 0 || strcmp(name, "join_room") == 0)
		fTemplates.FindMessage("room", &temp);
	return temp;
}


BObjectList<BMessage>
PurpleProtocol::Commands()
{
	return fCommands;
}


BObjectList<BMessage>
PurpleProtocol::UserPopUpItems()
{
	return BObjectList<BMessage>();
}


BObjectList<BMessage>
PurpleProtocol::ChatPopUpItems()
{
	return BObjectList<BMessage>();
}

BObjectList<BMessage>
PurpleProtocol::MenuBarItems()
{
	return BObjectList<BMessage>();
}


const char*
PurpleProtocol::Signature() const
{
	return fSignature.String();
}


const char*
PurpleProtocol::FriendlySignature() const
{
	return fFriendlySignature.String();
}


BBitmap*
PurpleProtocol::Icon() const
{
	return fIcon;
}


BPath
PurpleProtocol::AddOnPath()
{
	return fAddOnPath;
}


void
PurpleProtocol::SetAddOnPath(BPath path)
{
	fAddOnPath = path;
}


void
PurpleProtocol::SetAccountCachePath(BPath path)
{
	fAccountCachePath = path;
}


void
PurpleProtocol::SetAddOnCachePath(BPath path)
{
	fAddOnCachePath = path;
}


const char*
PurpleProtocol::GetName()
{
	return fName.String();
}


void
PurpleProtocol::SetName(const char* name)
{
	fName.SetTo(name);
}


uint32
PurpleProtocol::GetEncoding()
{
	return 0xffff;
}


ChatProtocolMessengerInterface*
PurpleProtocol::MessengerInterface() const
{
	return fMessenger;
}


void
PurpleProtocol::SendMessage(BMessage* msg)
{
	if (!msg)
		return;
	msg->AddString("protocol", fSignature);
	fMessenger->SendMessage(msg);
}


void
PurpleProtocol::_SendPrplMessage(BMessage* msg)
{
	msg->AddString("account_name", fName);
	msg->AddString("protocol", fSignature);
	if (fPrplMessenger->IsValid())
		fPrplMessenger->SendMessage(msg);
}


BMessage
PurpleProtocol::_RosterTemplate()
{
#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "PurpleProtocol ― Roster template"

	BMessage temp;
	BMessage id;
	id.AddString("name", "user_id");
	id.AddString("description", B_TRANSLATE("Username:"));
	id.AddString("error", B_TRANSLATE("You can't friend someone without a nick."));
	id.AddInt32("type", B_STRING_TYPE);
	temp.AddMessage("setting", &id);

	BMessage name;
	name.AddString("name", "user_name");
	name.AddString("description", B_TRANSLATE("Alias:"));
	name.AddInt32("type", B_STRING_TYPE);
	temp.AddMessage("setting", &name);

	return temp;
}


void
PurpleProtocol::_FindIcon(BPath* iconPath, directory_which finddir)
{
	BPath path;
	find_directory(finddir, &path);

	BString relPath = "pixmaps/pidgin/protocols/48/";
	relPath << fIconName << ".png";

	path.Append(relPath.String());
	if (BEntry(path.Path(), true).Exists() == true)
		iconPath->SetTo(path.Path());
}
