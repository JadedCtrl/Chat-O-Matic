/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2009-2011, Pier Luigi Fiorini. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _APP_PROTOCOL_H
#define _APP_PROTOCOL_H

#include <Messenger.h>
#include <ObjectList.h>
#include <Path.h>

class BBitmap;


// Chat-O-Matic protocol interface version
#define APP_VERSION_1_PRE_ALPHA_1		0x00000001
#define APP_VERSION_1_ALPHA_1			0x00000100

#define APP_VERSION 					APP_VERSION_1_ALPHA_1


class ChatProtocolMessengerInterface {
public:
	virtual status_t SendMessage(BMessage* message) = 0;
};

class ChatProtocol {
public:
	//! Messenger
	virtual status_t Init(ChatProtocolMessengerInterface*) = 0;

	//! Called before unloading from memory
	virtual status_t Shutdown() = 0;

	//! Process message
	virtual status_t Process(BMessage*) = 0;

	//! Change settings
	virtual status_t UpdateSettings(BMessage*) = 0;

	/*! Return a settings template
		Currently there are four: "account" (used when creating/editing
		the user's account), "create_room" & "join_room" (fairly self-evident),
		and "roster" (used when adding or editing a roster member. */
	virtual BMessage SettingsTemplate(const char* name) = 0;

	/*! Custom chat commands― archived ChatCommand objects
		Requires: String "_name", String "_desc", Bool "_proto",
				  Message "_msg", int32s "_argtype",
				  String "class" = "ChatCommand" */
	virtual BObjectList<BMessage> Commands() {
		return BObjectList<BMessage>();
	}

	/*! Custom menu items used in the userlist right-click menu.
		Archived BMenuItem with some extra slots.
		Requires: String "_label", Message "_msg", String "class" = "BMenuItem"
				  Bool "x_to_protocol", Bool "x_priority", int32 "x_perms",
				  int32 "x_target_perms", int32 "x_target_antiperms" */
	virtual BObjectList<BMessage> UserPopUpItems() {
		return BObjectList<BMessage>();
	}

	/*! Custom menu items used in the conversation-list right-click menu.
		Archived BMenuItem with some extra slots.
		Requires: String "_label", Message "_msg", String "class" = "BMenuItem"
				  Bool "x_to_protocol", int32 "x_perms" */
	virtual BObjectList<BMessage> ChatPopUpItems() {
		return BObjectList<BMessage>();
	}

	/*! Custom menubar items (in the "Protocol" menu).
		Archived BMenuItem with some extra slots.
		Requires: String "_label", Message "_msg", String "class" = "BMenuItem"
				  Bool "x_to_protocol" */
	virtual BObjectList<BMessage> MenuBarItems() {
		return BObjectList<BMessage>();
	}

	//! Protocol signature
	virtual const char* Signature() const = 0;

	//! Protocol name
	virtual const char* FriendlySignature() const = 0;

	//! Protocol icon
	virtual BBitmap* Icon() const { return NULL; }

	//! Pertinent paths
	virtual BPath AddOnPath() = 0;
	virtual void SetAddOnPath(BPath path) = 0;
	virtual void SetAccountCachePath(BPath path) { };
	virtual void SetAddOnCachePath(BPath path) { };

	//! Name of account file (leaf)
	virtual const char* GetName() = 0;
	virtual void SetName(const char* name) = 0;

	//! Preferred encoding of messages
	virtual uint32 GetEncoding() { return 0xffff; }

	//! Messenger interface used
	virtual ChatProtocolMessengerInterface* MessengerInterface() const = 0;
};

#endif	// _APP_PROTOCOL_H
