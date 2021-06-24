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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "PurpleProtocol.h"

#include <Application.h>
#include <Roster.h>

#include <ChatProtocolMessages.h>

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

	thread_id sender;

	char name[512] = { '\0' };
	char id[512] = { '\0' };
	receive_data(&sender, name, sizeof(char) * 512);
	receive_data(&sender, id, sizeof(char) * 512);

	return (ChatProtocol*)new PurpleProtocol(name, id);
}


int32
protocol_count()
{
	BMessage* msg = new BMessage(PURPLE_REQUEST_PROTOCOL_COUNT);
	msg->AddInt64("thread_id", find_thread(NULL));
	ensure_app_messenger()->SendMessage(msg);

	thread_id sender;
	int32 count = receive_data(&sender, NULL, 0);
	return count;
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
		kAppMessenger = new BMessenger("application/x-vnd.cardie.purple");
	}
	return kAppMessenger;
}


void
ensure_app()
{
	BRoster roster;
	if (roster.IsRunning("application/x-vnd.cardie.purple") == true)
		return;

	app_info aInfo;
	be_app->GetAppInfo(&aInfo);
	BPath protoPath(&aInfo.ref);
	protoPath.GetParent(&protoPath);
	protoPath.Append("protocols/purple");

	entry_ref protoRef;
	BEntry(protoPath.Path()).GetRef(&protoRef);
	roster.Launch(&protoRef);
}


status_t
connect_thread(void* data)
{
}


PurpleProtocol::PurpleProtocol(char name[512], char id[512])
	:
	fSignature(id),
	fFriendlySignature(name)
{
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
	thread_id thread = spawn_thread(connect_thread, "connect_thread",
		B_NORMAL_PRIORITY, (void*)this);

	if (thread < B_OK)
		return B_ERROR;

	resume_thread(thread);
	return B_OK;
}


BMessage
PurpleProtocol::SettingsTemplate(const char* name)
{
	return BMessage();
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
