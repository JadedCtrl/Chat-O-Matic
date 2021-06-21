/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _IRC_PROTOCOL_H
#define _IRC_PROTOCOL_H

#include <Path.h>
#include <Resources.h>
#include <String.h>

#include <libircclient.h>
#include <libirc_rfcnumeric.h>

#include <ChatProtocol.h>


status_t connect_thread(void* data);


class IrcProtocol : public ChatProtocol {
public:
						IrcProtocol();

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
	BResources fResources;
};


irc_callbacks_t	get_callbacks();

void	event_connect(irc_session_t* session, const char* event,
						const char* origin, const char** params,
						unsigned int count);

void	event_numeric(irc_session_t* session, unsigned int blah,
			const char* origin, const char** params, unsigned int count);

void	event_join(irc_session_t* session, const char* event,
			const char* origin, const char** params, unsigned int count);


#endif // _IRC_PROTOCOL_H
