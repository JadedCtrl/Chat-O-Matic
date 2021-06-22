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

#ifndef _PURPLE_PROTOCOL_H
#define _PURPLE_PROTOCOL_H

#include <Path.h>
#include <String.h>

#include <ChatProtocol.h>


class PurpleProtocol : public ChatProtocol {
public:
						PurpleProtocol();

	// ChatProtocol inheritance
	virtual	status_t	Init(ChatProtocolMessengerInterface* interface);
	virtual	status_t	Shutdown();

	virtual	status_t	Process(BMessage* msg);

	virtual	status_t	UpdateSettings(BMessage* msg);
	virtual	BMessage	SettingsTemplate(const char* name);

	virtual	BObjectList<BMessage> Commands();
	virtual	BObjectList<BMessage> UserPopUpItems();
	virtual	BObjectList<BMessage> ChatPopUpItems();
	virtual	BObjectList<BMessage> MenuBarItems();

	virtual	const char*	Signature() const;
	virtual const char*	FriendlySignature() const;

	virtual	BBitmap*	Icon() const;

	virtual	void		SetAddOnPath(BPath path);
	virtual	BPath		AddOnPath();

	virtual	const char*	GetName();
	virtual	void		SetName(const char* name);

	virtual	uint32		GetEncoding();

	virtual	ChatProtocolMessengerInterface*
						MessengerInterface() const;

private:
	ChatProtocolMessengerInterface* fMessenger;

	thread_id fServerThread;

	BString fName;
	BPath fAddOnPath;
};


#endif // _PURPLE_PROTOCOL_H
