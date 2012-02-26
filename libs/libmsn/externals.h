#ifndef __msn_externals_h__
#define __msn_externals_h__

/*
 * externals.h
 * libmsn
 *
 * Created by Meredydd Luff.
 * Refactored by Tiago Salem Herrmann.
 * Copyright (c) 2004 Meredydd Luff. All rights reserved.
 * Copyright (c) 2007 Tiago Salem Herrmann. All rights reserved
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

#include <config.h>
#include <buddy.h>
#include <util.h>

#include "libmsn_export.h"

namespace MSN
{
    class ListSyncInfo;
    /** The application should implement these callback functions to 
     * be able to receive notifications from the library.
     */
    class LIBMSN_EXPORT Callbacks
    {
    public:

        /** Asks the application that libmsn only must to be notified when the
         * socket become readable and/or writable.
         */
        virtual void registerSocket(void *sock, int read, int write, bool isSSL) = 0;

        /** Asks the application to never notify libmsn again about events in the socket.
         */
        virtual void unregisterSocket(void *sock) = 0;

        /** Asks the application to close the socket
         */
        virtual void closeSocket(void *sock) = 0;
        
        /** Allow your application to be notified about errors on library layer 
        */
        virtual void showError(MSN::Connection * conn, std::string msg) = 0;

        /** Notifies the application about a buddy status change.
         * msnobject is the object describing avatar file.
         * To get it you should call requestDisplayPicture() on a switchboard
         * connection with this user.
         */
        virtual void buddyChangedStatus(MSN::NotificationServerConnection * conn, MSN::Passport buddy, std::string friendlyname, MSN::BuddyStatus state, unsigned int clientID, std::string msnobject) = 0;

        /** Notifies the application about a user who went offline
         */
        virtual void buddyOffline(MSN::NotificationServerConnection * conn, MSN::Passport buddy) = 0;
        
        /** Allow your application to log some network traffic
         */
        virtual void log(int writing, const char* buf) = 0;
        
        /** Notifies the application that your friendly name now is 'friendlyname'.
         * Probably this is the reply to a previous setFriendlyName() call.
         */
        virtual void gotFriendlyName(MSN::NotificationServerConnection * conn, std::string friendlyname) = 0;

        /** Notifies the application that you have received your lists
         * (Address Book, allow list, block list, reverse list and pending list)
         * You must call completeConnection on notification server connection to
         * complete the initial process. An example can be found in msntest.cpp
         */
        virtual void gotBuddyListInfo(MSN::NotificationServerConnection * conn, MSN::ListSyncInfo * data) = 0;

        /** Notifies the application that a contact has updated his/her personal info.
         * Example: Current Song, personal messages..
         * Check all the possibilities in MSN::personalInfo struct
         */
        virtual void buddyChangedPersonalInfo(MSN::NotificationServerConnection * conn, MSN::Passport fromPassport, MSN::personalInfo pInfo) = 0;

        /** Notifies the application that one change was made to your
         * lists, and the current timestamp is lastChange.
         * This number should be used on initial process to specify
         * what version of your lists you already have. If the server
         * version is not the same, it will provide only the diff between your
         * version and the current one.
         * THE PARTIAL LIST FETCH IS NOT WORKING YET.
         */
        virtual void gotLatestListSerial(MSN::NotificationServerConnection * conn, std::string lastChange) = 0;

        /** This is a response to a previous sent GTC command.
         */
        virtual void gotGTC(MSN::NotificationServerConnection * conn, char c) = 0;


        /** This is a response to a previous sent BLP command.
         */
        virtual void gotBLP(MSN::NotificationServerConnection * conn, char c) = 0;
       
        /** Notifies your application that some list was modified by adding
         * some new contact. If ContactList is MSN::LST_RL, it means someone
         * added you to its Address Book. So at this point the application should 
         * prompt to the user about adding or blocking this new contact.
         */
        virtual void addedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport buddy, std::string friendlyname) = 0;

        /** Notifies your application that some list was modified by removing
         * some old contact. If ContactList is MSN::LST_RL, it means this contact
         * has removed you from its Address Book.
         */
        virtual void removedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport buddy) = 0;

        /** Notifies your application that a new group was created, or not, depending on 
         * the 'added' boolean variable. The application should track the groupId to permorm
         * actions with it.
         */
        virtual void addedGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupName, std::string groupId) = 0;

        /** Notifies your application that an old group was removed, or not, depending on
         * the 'removed' boolean variable.
         */
        virtual void removedGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupId) = 0;

        /** Notifies your application that an old group was renamed.
         */
        virtual void renamedGroup(MSN::NotificationServerConnection * conn, bool renamed, std::string newGroupName, std::string groupId) = 0;

        /** Notifies your application that a contact was added to a group, or not, depending on the
         * 'added' boolean variable.
         */
        virtual void addedContactToGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupId, std::string contactId) = 0;

        /** Notifies your application that a contact was removed from a group, or not, depending on the
         * 'removed' boolean variable.
         */
        virtual void removedContactFromGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupId, std::string contactId) = 0;

        /** Notifies your application that a contact was added to the Address Book, or not, depending on the
         * 'added' boolean variable.
         */
        virtual void addedContactToAddressBook(MSN::NotificationServerConnection * conn, bool added, std::string passport, std::string displayName, std::string guid) = 0;

        /** Notifies your application that a contact was removed from the Address Book, or not, depending on the
         * 'removed' boolean variable.
         */
        virtual void removedContactFromAddressBook(MSN::NotificationServerConnection * conn, bool removed, std::string contactId, std::string passport) = 0;

        /** Notifies your application that a contact was enabled on Address Book, or not, depending on the
         * 'enabled' boolean variable.
         */
        virtual void enabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool enabled, std::string contactId, std::string passport) = 0;

        /** Notifies your application that a contact was disabled on Address Book, or not, depending on the
         * 'disabled' boolean variable.
         */
        virtual void disabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool disabled, std::string contactId) = 0;

        /** Notifies your application that you got a new switchboard connection
         */
        virtual void gotSwitchboard(MSN::SwitchboardServerConnection * conn, const void * tag) = 0;

        /** Notifies your application that a contact joined to a switchboard connection
         */
        virtual void buddyJoinedConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string friendlyname, int is_initial) = 0;

        /** Notifies your application that a contact left a switchboard connection
         */
        virtual void buddyLeftConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy) = 0;

        /** Notifies your application that received an instant message
         */
        virtual void gotInstantMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string friendlyname, MSN::Message * msg) = 0;

        /** Notifies your application that the "trID" message was received by the other contact
         */
        virtual void gotMessageSentACK(MSN::SwitchboardServerConnection * conn, int trID) = 0;

        /** Notifies your application that received an emoticon notification. 
         * To get it you should call requestEmoticon() and pass msnobject to it
         */
        virtual void gotEmoticonNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string alias, std::string msnobject) = 0;
            
        /** Notifies your application that a message could not be delivered
         */
        virtual void failedSendingMessage(MSN::Connection * conn) = 0;

        /** Notifies your application that you got a nudge
         */
        virtual void gotNudge(MSN::SwitchboardServerConnection * conn, MSN::Passport username) = 0;

        /** Notifies your application that you got a new voice clip.
         * To get it you should call requestVoiceClip() and pass msnobject to it
         */
        virtual void gotVoiceClipNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string msnobject) = 0;

        /** Notifies your application that you got a new wink.
         * To get it you should call requestWink() and pass msnobject to it
         */
        virtual void gotWinkNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string msnobject) = 0;

        /** Notifies your application that you got a new Ink.
         */
        virtual void gotInk(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string image) = 0;

        /** Notifies your application that you got an action.
         */
        virtual void gotActionMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string message) = 0;

        /** Notifies your application that a buddy is typing.
         */
        virtual void buddyTyping(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string friendlyname) = 0;
            
        /** Notifies your application that you got an initial email notification.
         */
        virtual void gotInitialEmailNotification(MSN::NotificationServerConnection * conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders) = 0;
            
        /** Notifies your application that you got an email notification.
         */
        virtual void gotNewEmailNotification(MSN::NotificationServerConnection * conn, std::string from, std::string subject) = 0;
            
        /** Notifies your application about a progress of a file transfer.
         */
        virtual void fileTransferProgress(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, long long unsigned transferred, long long unsigned total) = 0;
            
        /** Notifies your application that some file transfer has failed
         */
        virtual void fileTransferFailed(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, MSN::fileTransferError error) = 0;
            
        /** Notifies your application that some file transfer has succeeded.
         */
        virtual void fileTransferSucceeded(MSN::SwitchboardServerConnection * conn, unsigned int sessionID) = 0;

        /** Notifies your application that the other contact replied a file transfer
         * invitation. if "response" is true, then the other contact has accepted,
         * if false, rejected.
         */
        virtual void fileTransferInviteResponse(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, bool response) = 0;
            
        /** Notifies your application that the other contact sent you a voice clip
         */
        virtual void gotVoiceClipFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file) = 0;
 
        /** Notifies your application that the other contact sent you an emoticon
         */
        virtual void gotEmoticonFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string alias, std::string file) = 0;

        /** Notifies your application that the other contact sent a wink
         */
        virtual void gotWinkFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file) = 0;

        /** Notifies your application that there is a new connection
         */
        virtual void gotNewConnection(MSN::Connection * conn) = 0;
            
        /** Notifies your application that you got a new OIM list.
         */
        virtual void gotOIMList(MSN::NotificationServerConnection * conn, std::vector<MSN::eachOIM> OIMs) = 0;

        /** Notifies your application that you got (or not)  one previously requested OIM.
         */
        virtual void gotOIM(MSN::NotificationServerConnection * conn, bool success, std::string id, std::string message) = 0;

        /** Notifies your application if a previously sent OIM was delivered.
         */
        virtual void gotOIMSendConfirmation(MSN::NotificationServerConnection * conn, bool success, int id) = 0;

        /** Notifies your application if a previously request to delete one OIM was successful.
         */
        virtual void gotOIMDeleteConfirmation(MSN::NotificationServerConnection * conn, bool success, std::string id) = 0;

        /** Notifies your application that you got a new display picture from "passaport"
         */
        virtual void gotContactDisplayPicture(MSN::SwitchboardServerConnection * conn, MSN::Passport passport, std::string filename ) = 0;

        /** Notifies your application that the connection "conn" is ready
         */
        virtual void connectionReady(MSN::Connection * conn) = 0;

        /** Notifies your application that the connection "conn" is closed
         */
        virtual void closingConnection(MSN::Connection * conn) = 0;

        /** Notifies your application that a contact has changed his/her status
         * ex: Online, Away, Out to Lunch.. 
         */
        virtual void changedStatus(MSN::NotificationServerConnection * conn, MSN::BuddyStatus state) = 0;

        /** Asks your application to create a socket with "server" at port "port".
         * This function must return a socket reference that will be used for 
         * getDataFromSocket() and writeDataToSocket()
         */
        virtual void * connectToServer(std::string server, int port, bool *connected, bool isSSL = false) = 0;

        /** Notifies your application that someone is trying to send you a file.
         */
        virtual void askFileTransfer(MSN::SwitchboardServerConnection *conn, MSN::fileTransferInvite ft) = 0;

        virtual int listenOnPort(int port) = 0;

        virtual std::string getOurIP() = 0;

        virtual std::string getSecureHTTPProxy() = 0;

        virtual int getSocketFileDescriptor (void *sock) = 0;

        /** Asks your application to get @c size bytes of data available in @p sock
         * and store them in @p data.
         * It must return the real size written to @p data
         */
        virtual size_t getDataFromSocket (void *sock, char *data, size_t size) = 0;

        /** Asks your application to write @c size bytes from @p data to @p sock.
         * It must return the real size written to *sock
         */
        virtual size_t writeDataToSocket (void *sock, char *data, size_t size) = 0;

        virtual void gotInboxUrl (MSN::NotificationServerConnection * /*conn*/, MSN::hotmailInfo /*info*/) {};
    };
}
#endif
