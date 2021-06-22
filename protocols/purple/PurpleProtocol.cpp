/*
 Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "PurpleProtocol.h"

#include <cstdio>

#include <StringList.h>

#include <ChatProtocolMessages.h>


PurpleProtocol::PurpleProtocol()
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
	return "purple";
}


const char*
PurpleProtocol::FriendlySignature() const
{
	return "Purple";
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
