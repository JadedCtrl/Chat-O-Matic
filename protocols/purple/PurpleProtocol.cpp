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
#include <Roster.h>

#include <ChatProtocolMessages.h>

#include "Purple.h"
#include "PurpleMessages.h"


BMessenger* kAppMessenger = NULL;


ChatProtocol*
protocol_at(int32 i)
{
	BMessenger* msgr = ensure_app_messenger();

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
	BMessage* msg = new BMessage(PURPLE_REQUEST_PROTOCOL_COUNT);
	msg->AddInt64("thread_id", find_thread(NULL));
	ensure_app_messenger()->SendMessage(msg);

	thread_id sender;
	return receive_data(&sender, NULL, 0);
}


const char*
signature()
{
	return "purple";
}


const char*
friendly_signature()
{
	return "Purple";
}


uint32
version()
{
	return APP_VERSION_1_PRE_ALPHA_1;
}


BMessenger*
ensure_app_messenger()
{
	if (kAppMessenger == NULL || kAppMessenger->IsValid() == false) {
		ensure_app();
		kAppMessenger = new BMessenger(PURPLE_SIGNATURE);
	}
	return kAppMessenger;
}


void
ensure_app()
{
	BRoster roster;
	if (roster.IsRunning(PURPLE_SIGNATURE) == true)
		return;

	app_info aInfo;
	be_app->GetAppInfo(&aInfo);
	BPath protoPath(&aInfo.ref);
	protoPath.GetParent(&protoPath);
	protoPath.Append("protocols/purple");

	entry_ref protoRef;
	BEntry(protoPath.Path()).GetRef(&protoRef);
	roster.Launch(&protoRef);
	snooze(100000);
}


status_t
connect_thread(void* data)
{
	PurpleProtocol* protocol = (PurpleProtocol*)data;
	while (true)
		protocol->SendMessage(new BMessage(receive_message()));
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
	fSettingsTemplate(settings)
{
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
	BMessage* disconnect = new BMessage(PURPLE_REQUEST_DISCONNECT);
	_SendPrplMessage(disconnect);

	kill_thread(fBirdThread);
	return B_OK;
}


status_t
PurpleProtocol::Process(BMessage* msg)
{
	return B_OK;
}


status_t
PurpleProtocol::UpdateSettings(BMessage* msg)
{
	ensure_app();
	fPrplMessenger = new BMessenger(PURPLE_SIGNATURE);
	msg->what = PURPLE_CONNECT_ACCOUNT;
	_SendPrplMessage(msg);

	thread_id thread = spawn_thread(connect_thread, "bird_superiority",
		B_NORMAL_PRIORITY, (void*)this);

	if (thread < B_OK)
		return B_ERROR;

	BMessage* account = new BMessage(PURPLE_REGISTER_THREAD);
	account->AddInt64("thread_id", thread);
	_SendPrplMessage(account);

	resume_thread(thread);
	return B_OK;
}


BMessage
PurpleProtocol::SettingsTemplate(const char* name)
{
	return fSettingsTemplate;
}


BObjectList<BMessage>
PurpleProtocol::Commands()
{
	return BObjectList<BMessage>();
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
	return NULL;
}


void
PurpleProtocol::SetAddOnPath(BPath path)
{
	fAddOnPath = path;
}


BPath
PurpleProtocol::AddOnPath()
{
	return fAddOnPath;
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
