/*
 * Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>
 * All rights reserved. Distributed under the terms of the MIT license.
 */
#ifndef _IRC_PROTOCOL_H
#define _IRC_PROTOCOL_H

#include <String.h>
#include <StringList.h>

#include <libsupport/KeyMap.h>

#include <ChatProtocol.h>


typedef KeyMap<BString, BString> StringMap;


class BSocket;
class BDataIO;


class IrcProtocol : public ChatProtocol {
public:
						IrcProtocol();
						~IrcProtocol();

	// ChatProtocol inheritance
	virtual	status_t	Init(ChatProtocolMessengerInterface* interface);
	virtual	status_t	Shutdown();

	virtual	status_t	UpdateSettings(BMessage* settings);

	virtual	status_t	Process(BMessage* msg);

	virtual	BMessage	SettingsTemplate(const char* name);

	virtual	const char*	Signature() const { return "irc"; }
	virtual const char*	FriendlySignature() const { return "IRC"; }

	virtual BBitmap*	Icon() const;

	virtual	void		SetAddOnPath(BPath path) { fAddOnPath = path; }
	virtual	BPath		AddOnPath() { return fAddOnPath; }

	virtual	const char*	GetName() { return fName; }
	virtual	void		SetName(const char* name) { fName = name; }

	virtual	ChatProtocolMessengerInterface*
						MessengerInterface() const { return fMessenger; }

	// IRC
			status_t	Loop();

	BMessage* fSettings;

private:
			void		_ProcessLine(BString line);
			void		_ProcessNumeric(int32 numeric, BString sender,
							BStringList params);
			void		_ProcessNumericError(int32 numeric, BString sender,
							BStringList params);
			void		_ProcessCommand(BString command, BString sender,
							BStringList params);

			void		_MakeReady(BString nick, BString ident);

			BString		_LineSender(BStringList words);
			BString		_LineCode(BStringList words);
			BStringList	_LineParameters(BStringList words, BString line);

			void		_SendMsg(BMessage* msg);
			void		_SendIrc(BString cmd);

			// Used with "nick!ident"-formatted strings
			BString 	_SenderNick(BString sender);
			BString 	_SenderIdent(BString sender);

			BString		_IdentNick(BString ident);

			bool		_IsChannelName(BString name);

			// Read a data stream until newline found; if data found past
			// newline, append to given buffer for later use
			BString		_ReadUntilNewline(BDataIO* data, BString* extraBuffer);
			// Trim given string until newline hit, return trimmed part
			BString		_TrimStringToNewline(BString* str);

			// GUI templates
			BMessage	_AccountTemplate();
			BMessage	_RoomTemplate();

	BSocket* fSocket;
	BString fRemainingBuf;
	thread_id fRecvThread;

	BString fNick;
	const char* fUser;
	BString fIdent;
	BString fPartText;

	// WHOREPLY is requested by the add-on to populate the user-list, but the
	// user might also use the /who command― if the user does, this is true
	bool fWhoRequested;
	bool fWriteLocked;

	StringMap fIdentNicks; // User ident → nick

	BPath fAddOnPath;
	BString fName;
	ChatProtocolMessengerInterface* fMessenger;
	bool fReady;
};

#endif // _IRC_PROTOCOL_H
