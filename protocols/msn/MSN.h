/*
 * Copyright 2010, Dario Casalinuovo. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef IMKIT_MSNP_H
#define IMKIT_MSNP_H

#include <sys/poll.h>

#include <Messenger.h>
#include <OS.h>
#include <Path.h>
#include <String.h>

#include <libsupport/List.h>
#include <CayaProtocol.h>
#include <CayaConstants.h>
#include <CayaProtocolMessages.h>

#include <msn.h>

using namespace std;


class MSNP : public MSN::Callbacks, public CayaProtocol {
public:

					MSNP();
		virtual 	~MSNP();

		virtual 	status_t Init(CayaProtocolMessengerInterface*);

		virtual 	status_t Shutdown();

		virtual 	status_t Process(BMessage *);
		
		virtual 	const char * Signature() const;
		virtual 	const char * FriendlySignature() const;

		virtual 	status_t UpdateSettings(BMessage *);

		virtual 	uint32 GetEncoding();

		virtual 	CayaProtocolMessengerInterface* MessengerInterface() const { return fServerMsgr; }
		virtual 	MSN::NotificationServerConnection* GetConnection() const { return fMainConnection; }
		virtual		uint32	Version() const;

		void 		Error(const char* message, const char* who);
		void 		Progress(const char* id, const char* message, float progress);
		void 		SendContactInfo(MSN::Buddy buddy);
		void		MessageFromBuddy(const char* mess, const char* id);

		void		RequestBuddyIcon(string msnobject, MSN::Passport buddy);
		void		RequestBuddyIcon(MSN::SwitchboardServerConnection* conn, string msnobject, const char* buddy);
		void		SendBuddyIcon(string filename, string passport);
		bool		CheckAvatar(string msnobject, string passport);
		string		GetObject(string passport);
		string		GetFilename(string name);
		const char*	FindSHA1D(string msnobject);
private:
		BPath		fCachePath;
		thread_id   fPollThread;
		bool		fLogged;
		BString		fServer;
		string		fUsername;
		BString		fPassword;
		uint		fClientID;
		string		fNickname;

		bool		fSettings;
	unsigned int 	fID;

		CayaProtocolMessengerInterface*	fServerMsgr;
		MSN::NotificationServerConnection* fMainConnection;

		List<MSN::Buddy> fBuddyList;
		List<pair<string, MSN::SwitchboardServerConnection*>*> fSwitchboardList;

		List<pair<string, string>*> fAvatarQueue;
		List<pair<string, string>*> fAvatarDone;

// LibMSN Callbacks :
		virtual void registerSocket(void* s, int read, int write, bool isSSL);

		virtual void unregisterSocket(void* s);
    
		virtual void closeSocket(void* s);

		virtual void showError(MSN::Connection* conn, std::string msg);

		virtual void buddyChangedStatus(MSN::NotificationServerConnection* conn,
						MSN::Passport buddy, std::string friendlyname, MSN::BuddyStatus state,
						unsigned int clientID, std::string msnobject);

		virtual void buddyOffline(MSN::NotificationServerConnection* conn,
						MSN::Passport buddy);

		virtual void log(int writing, const char* buf);

		virtual void buddyChangedPersonalInfo(MSN::NotificationServerConnection* conn,
						MSN::Passport fromPassport, MSN::personalInfo);

		virtual void gotFriendlyName(MSN::NotificationServerConnection* conn,
						std::string friendlyname);

		virtual void gotBuddyListInfo(MSN::NotificationServerConnection* conn,
						MSN::ListSyncInfo* data);

		virtual void gotLatestListSerial(MSN::NotificationServerConnection* conn,
						std::string lastChange);

		virtual void gotGTC(MSN::NotificationServerConnection* conn, char c);

		virtual void gotBLP(MSN::NotificationServerConnection* conn, char c);

		virtual void addedListEntry(MSN::NotificationServerConnection* conn,
						MSN::ContactList list, MSN::Passport buddy, std::string friendlyname);

		virtual void removedListEntry(MSN::NotificationServerConnection* conn,
						MSN::ContactList list, MSN::Passport buddy);

		virtual void addedGroup(MSN::NotificationServerConnection* conn,
						bool added, std::string groupName, std::string groupID);

		virtual void removedGroup(MSN::NotificationServerConnection* conn,
						bool removed, std::string groupID);

		virtual void renamedGroup(MSN::NotificationServerConnection* conn,
						bool renamed, std::string newGroupName, std::string groupID);

		virtual void addedContactToGroup(MSN::NotificationServerConnection* conn, 
						bool added, std::string groupId, std::string contactId);

		virtual void removedContactFromGroup(MSN::NotificationServerConnection* conn,
						bool removed, std::string groupId, std::string contactId);

		virtual void addedContactToAddressBook(MSN::NotificationServerConnection* conn,
						bool added, std::string passport, std::string displayName, std::string guid);

		virtual void removedContactFromAddressBook(MSN::NotificationServerConnection* conn,
						bool removed, std::string contactId, std::string passport);

		virtual void enabledContactOnAddressBook(MSN::NotificationServerConnection* conn,
						bool enabled, std::string contactId, std::string passport);
		virtual void disabledContactOnAddressBook(MSN::NotificationServerConnection* conn,
						bool disabled, std::string contactId);

		virtual void gotSwitchboard(MSN::SwitchboardServerConnection* conn, const void* tag);

		virtual void buddyJoinedConversation(MSN::SwitchboardServerConnection* conn,
						MSN::Passport buddy, std::string friendlyname, int is_initial);

		virtual void buddyLeftConversation(MSN::SwitchboardServerConnection* conn,
						MSN::Passport buddy);

		virtual void gotInstantMessage(MSN::SwitchboardServerConnection* conn,
						MSN::Passport buddy, std::string friendlyname, MSN::Message* msg);

		virtual void gotMessageSentACK(MSN::SwitchboardServerConnection * conn, int trID);

		virtual void gotEmoticonNotification(MSN::SwitchboardServerConnection* conn,
						MSN::Passport buddy, std::string alias, std::string msnobject);

		virtual void failedSendingMessage(MSN::Connection * conn);

		virtual void buddyTyping(MSN::SwitchboardServerConnection* conn,
						MSN::Passport buddy, std::string friendlyname);

		virtual void gotNudge(MSN::SwitchboardServerConnection* conn, MSN::Passport from);

		virtual void gotVoiceClipNotification(MSN::SwitchboardServerConnection* conn,
						MSN::Passport from, std::string msnobject);

		virtual void gotWinkNotification(MSN::SwitchboardServerConnection* conn,
						MSN::Passport from, std::string msnobject);

		virtual void gotInk(MSN::SwitchboardServerConnection* conn, 
						MSN::Passport from, std::string image);

		virtual void gotVoiceClipFile(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID, std::string file);

		virtual void gotEmoticonFile(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID, std::string alias, std::string file);

		virtual void gotWinkFile(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID, std::string file);

		virtual void gotActionMessage(MSN::SwitchboardServerConnection* conn,
						MSN::Passport username, std::string message);

		virtual void gotInitialEmailNotification(MSN::NotificationServerConnection* conn,
						int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders);

		virtual void gotNewEmailNotification(MSN::NotificationServerConnection* conn,
						std::string from, std::string subject);

		virtual void fileTransferProgress(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID, long long unsigned transferred, long long unsigned total);

		virtual void fileTransferFailed(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID, MSN::fileTransferError error);

		virtual void fileTransferSucceeded(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID);

		virtual void fileTransferInviteResponse(MSN::SwitchboardServerConnection* conn,
						unsigned int sessionID, bool response);

		virtual void gotNewConnection(MSN::Connection* conn);

		virtual void gotOIMList(MSN::NotificationServerConnection* conn,
						std::vector<MSN::eachOIM> OIMs);

		virtual void gotOIM(MSN::NotificationServerConnection* conn,
						bool success, std::string id, std::string message);

		virtual void gotOIMSendConfirmation(MSN::NotificationServerConnection* conn,
						bool success, int id);

		virtual void gotOIMDeleteConfirmation(MSN::NotificationServerConnection* conn,
						bool success, std::string id);

		virtual void gotContactDisplayPicture(MSN::SwitchboardServerConnection* conn,
						MSN::Passport passport, std::string filename);

		virtual void closingConnection(MSN::Connection* conn);

		virtual void changedStatus(MSN::NotificationServerConnection* conn,
						MSN::BuddyStatus state);

		virtual void* connectToServer(std::string server, int port,
						bool* connected, bool isSSL);

		virtual void connectionReady(MSN::Connection* conn);

		virtual void askFileTransfer(MSN::SwitchboardServerConnection*conn,
						MSN::fileTransferInvite ft);

		virtual int listenOnPort(int port);

		virtual std::string getOurIP();

		virtual int getSocketFileDescriptor (void* sock);

		virtual size_t getDataFromSocket (void* sock, char* data, size_t size);

		virtual size_t writeDataToSocket (void* sock, char* data, size_t size);

		virtual std::string getSecureHTTPProxy();
		virtual void gotInboxUrl (MSN::NotificationServerConnection *conn,
						MSN::hotmailInfo info);
};

extern const char* kProtocolName;
extern const char* kProtocolSignature;

#endif	// IMKIT_MSNP_H
