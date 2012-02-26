#ifndef __msn_notificationserver_h__
#define __msn_notificationserver_h__

/*
 * notificationserver.h
 * libmsn
 *
 * Created by Mark Rowe on Mon Mar 22 2004.
 * Refactored by Tiago Salem Herrmann on 08/2007.
 * Copyright (c) 2004 Mark Rowe. All rights reserved.
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

#include <connection.h>
#include <authdata.h>
#include <errorcodes.h>
#include <buddy.h>
#include <passport.h>
#include <stdexcept>
#include <externals.h>
#include <msnobject.h>
#include <soap.h>
#include <cassert>
#include <sys/types.h>

#include "libmsn_export.h"

#ifdef _WIN32
typedef unsigned uint;
#endif

namespace MSN
{    
    class SwitchboardServerConnection;
    
    /** Contains information about synchronising the contact list with the server.
     */
    class LIBMSN_EXPORT ListSyncInfo
    {
public:
        /** Constants specifying the status of the list synchronisation.
         */
        enum SyncProgress
        {
            LST_AB = 1,        /**< Address book has been received. */
            LST_AL = 2,        /**< Allow list has been received.   */
            LST_BL = 4,        /**< Block list has been received.   */
            LST_RL = 8,        /**< Reverse list has been received. */
            LST_PL = 16,       /**< Pending list has been received. */
            COMPLETE_BLP = 32  /**< @c BLP has been received.       */
        };
        
        
        /** Treat unknown users as if appearing on this list when they attempt
         *  to initiate a switchboard session.
         */
        enum PrivacySetting
        {
            ALLOW = 'A', /**< Any user can see our status, except those in Block List. */
            BLOCK = 'B' /**< Nobody can see our status, except those in Allow List. */
        };
        
        /** Action to take when a new user appears on our reverse list.
         */
        enum NewReverseListEntryAction
        {
            PROMPT = 'A',
            DONT_PROMPT = 'N'
        };


        /** A list of people who's statuses we are wish to be notified about.
         */
        std::map<std::string, Buddy *> contactList;
        
        std::string myDisplayName;
       
        std::map<std::string, Group> groups;
        
        /** The progress of the sync operation.
         */
        unsigned int progress;
        
        unsigned int usersRemaining, groupsRemaining;

        /** Last change on Address Book and lists timestamp 
             */
        std::string lastChange;

        /** Specifies the default list for non-buddies to be treated as
         *  appearing in when attempting to invite you into a switchboard setting.
         *
         *  This corresponds to the @c BLP command in the MSN protocol.
         */
        char privacySetting;
        
        /** Specifies whether the user should be prompted when a new buddy
         *  appears on their reverse list.
         *
         *  This corresponds to the @c GTC command in the MSN protocol.
         */
        char reverseListPrompting;
        
        ListSyncInfo(std::string lastChange_) : 
            progress(0), lastChange(lastChange_), 
            privacySetting(ListSyncInfo::ALLOW), reverseListPrompting(ListSyncInfo::PROMPT) {};
    };
    
    // Intermediate steps in connection:
    class LIBMSN_EXPORT connectinfo
    {
public:
        Passport username;
        std::string password;
        std::string cookie;
        
        connectinfo(const Passport & username_, const std::string & password_) : username(username_), password(password_), cookie("") {};
    };    
        
    /** Represents a connection to a MSN notification server.
     *
     *  The MSN notification server is responsible for dealing with the contact
     *  list, online status, and dispatching message requests or invitations to
     *  or from switchboard servers.
     */
    class LIBMSN_EXPORT NotificationServerConnection : public Connection
    {
private:
        typedef void (NotificationServerConnection::*NotificationServerCallback)(std::vector<std::string> & args, int trid, void *);

        std::string token;
        class AuthData : public ::MSN::AuthData
        {
public:
            std::string password;
            
            AuthData(const Passport & passport_,
                     const std::string & password_) : 
                ::MSN::AuthData(passport_), password(password_) {} ;
        };
        NotificationServerConnection::AuthData auth;
        int synctrid;

public:
        MSNObject msnobj;
       
        /** Create a NotificationServerConnection with the specified @a username and 
         *  @a password.
         */
        NotificationServerConnection(Passport username, std::string password, Callbacks & cb);
        
        virtual ~NotificationServerConnection();
        virtual void dispatchCommand(std::vector<std::string> & args);
        
        /** Return a list of SwitchboardServerConnection's that have been started
         *  from this NotificationServerConnection.
         */
        const std::vector<SwitchboardServerConnection *> & switchboardConnections();
        
        /* Add a SwitchboardServerConnection to the list of connections that have
         *  been started from this connection.
         */
        void addSwitchboardConnection(SwitchboardServerConnection *);

        /* Add the @p Soap object to the list of connections that have
         *  been started from this connection.
         */
        void addSoapConnection(Soap *);

        /* Remove a SwitchboardServerConnection from the list of connections that have
         *  been started from this connection.
         */
        void removeSwitchboardConnection(SwitchboardServerConnection *);

        /* Remove the @p Soap object from the list of connections that have
         *  been started from this connection.
         */
        void removeSoapConnection(Soap *);
        
        /** Return a connection that is associated with @a fd.
         *
         *  If @a fd is equal to @p sock, @c this is returned.  Otherwise
         *  connectionWithSocket is sent to each SwitchboardServerConnection
         *  or Soap connections until a match is found.
         *  
         *  @return  The matching connection, if found.  Otherwise, @c NULL.
         */
        Connection *connectionWithSocket(void *sock);
        
        /** Return a SwitchboardServerConnection that has exactly one user whose
         *  username is @a username.
         *
         *  @return  The matching SwitchboardServerConnection, or @c NULL.
         */
        SwitchboardServerConnection *switchboardWithOnlyUser(Passport username);
        
        /** @name Action Methods
         *
         *  These methods all perform actions on the notification server.
         */
        /** @{ */
 
        /** Set our capabilities. This is the sum of MSNClientInformationFields enum fields
         */
        void setCapabilities(uint m_clientId);

        void disconnectNS();

        /** Set our online state to @a state and our capabilites to @a clientID.
         */
        void setState(BuddyStatus state, uint clientID);
        
        /** Set Black List policy
        */
        void setBLP(char setting);

        /** It must be called to complete the connection. Add all your contacts 
         * to allContacts, (both Forward, allow and block lists). The integer value
         * of allContacts is the sum of all lists this contact is present. For example:
         * If the contact is both on your allow and forward list, the number must be
         * 3 (2+1). 
         */
        void completeConnection(std::map<std::string,int > & allContacts, void *data);

        /** Set our friendly name to @a friendlyName.
         *
         *  @param  friendlyName  Maximum allowed length is 387 characters after URL-encoding.
         */
        void setFriendlyName(std::string friendlyName, bool updateServer = false) throw (std::runtime_error);

        /** Points to our display picture. It must be a png file, size: 96x96.
         *  @param  filename  Path to the PNG file to be sent as avatar of this contact
         */
        bool change_DisplayPicture(std::string filename);

        /** Sets our personal info. Current media, personal message...
        */
        void setPersonalStatus(personalInfo pInfo);

        /** Add buddy named @a buddyName to the list named @a list.
         */
        void addToList(MSN::ContactList list, Passport buddyName);
        
        /** Remove buddy named @a buddyName from the list named @a list.
         */
        void removeFromList(MSN::ContactList list, Passport buddyName);

        /** Add contact @a buddyName to address book with @a displayName
         * Automatically adds to allow list too.
         */
        void addToAddressBook(Passport buddyName, std::string displayName);

        /** Delete contact @a passport with @a contactId from address book.
         */
        void delFromAddressBook(std::string contactId, std::string passport);

        /** Enable a contact on address book. You will use it when you have disabled 
         * a contact before. This contact is still on your Address Book, but it is not
         * a messenger contact. This function will turn this contact into a messenger
         * contact again.
         */
        void enableContactOnAddressBook(std::string contactId, std::string passport);

        /** Do not delete the contact, just disable it.
         */
        void disableContactOnAddressBook(std::string contactId, std::string passport);

        /** Block a contact. This user won't be able to see your status anymore.
         */
        void blockContact(Passport buddyName);

        /** Unblock a contact. This user will be able to see your status again.
         */
        void unblockContact(Passport buddyName);

        /** Add a contact @a contactId to the group @a groupId .
         */
        void addToGroup(std::string groupId, std::string contactId);

        /** Remove a contact from a group.
         */
        void removeFromGroup(std::string groupId, std::string contactId);
        
        /** Add a new group.
         */
        void addGroup(std::string groupName);

        /** Remove a group.
         */
        void removeGroup(std::string groupId);

        /** Rename a group.
         */
        void renameGroup(std::string groupId, std::string newGroupName);
        
        /** Request the server side buddy list.
         *
         *  @param  lastChange  if @a version is specified the server will respond with
         *                      the changes necessary to update the list to the latest
         *                      version.  Otherwise the entire list will be sent.
         */
        void synchronizeContactList(std::string lastChange="0");
        
        /** Send a 'keep-alive' ping to the server.
         */
        void sendPing();
        
        /** Request a switchboard connection.
         */
        void requestSwitchboardConnection(const void *tag);

        /** Retrieve the Offline Instant Message identified by id
         */
        void get_oim(std::string id, bool markAsRead);

        /** Erase the Offline Instant Message identified by id
         */
        void delete_oim(std::string id);

        /** Send an Offline Instant Message
         */
        void send_oim(Soap::OIM oim);
    
        void getInboxUrl();
 
        /* when we have to send more than 1 ADL command, we need to keep this here to track */
        std::list<std::string> adl_packets;

        /* Our current Display Name */
        std::string myDisplayName;

        /* Our passport */
        std::string myPassport;

        /* Sum of capabilities of the user */
        uint m_clientId;
        
        char bplSetting;

        /* Our IP number reported by notification server */
        std::string server_reported_ip;

        /* Our TCP source port reported by notification server */
        std::string server_reported_port;

        std::string login_time;

        std::string MSPAuth;

        std::string sid;

        std::string kv;

        /* 1 if our email is verified, 0 if not */
        std::string server_email_verified;

        /* Says if we are direct connected based on server's report */
        bool direct_connection;
       
        virtual void connect(const std::string & hostname, unsigned int port);

        virtual void connect(const std::string & hostname, unsigned int port, 
                const Passport & username,  
                const std::string & password);

        virtual void disconnect();
        
        virtual void addCallback(NotificationServerCallback cb, int trid, void *data);
        
        virtual void removeCallback(int trid);
        
        virtual void socketConnectionCompleted();
        
        enum NotificationServerState
        {
            NS_DISCONNECTED,
            NS_CONNECTING,
            NS_CONNECTED,
            NS_SYNCHRONISING,
            NS_ONLINE
        };

        connectinfo *info;
        NotificationServerState connectionState() const { return this->_connectionState; };
        Callbacks & externalCallbacks;
        virtual NotificationServerConnection *myNotificationServer() { return this; };
        void gotTickets(Soap & soapConnection, std::vector<MSN::Soap::sitesToAuth> sitesToAuthList);
        void gotLists(Soap &soapConnection);
        void gotAddressBook(Soap &soapConnection);
        void gotOIM(Soap & soapConnection, bool success, std::string id, std::string message);
        void gotOIMLockkey(Soap & soapConnection, std::string lockkey);
        void gotOIMSendConfirmation(Soap & soapConnection, int id, bool sent);
        void gotOIMDeleteConfirmation(Soap & soapConnection, std::string id, bool deleted);
        void gotSoapMailData(Soap & soapConnection, std::string maildata);
        void gotChangeDisplayNameConfirmation(Soap & soapConnection, std::string displayName, bool changed);
        void gotDelContactFromGroupConfirmation(Soap & soapConnection,
                    bool deleted,
                    std::string newVersion,
                    std::string groupId,
                    std::string contactId);

        void gotAddContactToGroupConfirmation(Soap & soapConnection,
                    bool added,
                    std::string newVersion,
                    std::string groupId,
                    std::string contactId);

        void gotAddGroupConfirmation(Soap & soapConnection,
                    bool added,
                    std::string newVersion,
                    std::string groupName,
                    std::string groupId);

        void gotDelGroupConfirmation(Soap & soapConnection,
                    bool removed,
                    std::string newVersion,
                    std::string groupId);

        void gotRenameGroupConfirmation(Soap & soapConnection,
                    bool renamed,
                    std::string newVersion,
                    std::string newGroupName,
                    std::string groupId);

        void gotAddContactToAddressBookConfirmation(Soap & soapConnection,
                    bool added,
                    std::string newVersion,
                    std::string passport,
                    std::string displayName,
                    std::string guid);

        void gotDelContactFromAddressBookConfirmation(Soap & soapConnection,
                    bool removed,
                    std::string newVersion,
                    std::string contactId,
                    std::string passport);

        void gotEnableContactOnAddressBookConfirmation(Soap & soapConnection,
                    bool enabled,
                    std::string newVersion,
                    std::string contactId,
                    std::string passport);

        void gotDisableContactOnAddressBookConfirmation(Soap & soapConnection,
                    bool disabled,
                    std::string newVersion,
                    std::string contactId,
                    std::string passport);

        void gotAddContactToListConfirmation(Soap & soapConnection,
                    bool added,
                    std::string newVersion,
                    std::string passport,
                    MSN::ContactList list);

        void gotDelContactFromListConfirmation(Soap & soapConnection,
                    bool deleted,
                    std::string newVersion,
                    std::string passport,
                    MSN::ContactList list);

protected:
        virtual void handleIncomingData();
        NotificationServerState _connectionState;
        
        void setConnectionState(NotificationServerState s) { this->_connectionState = s; };
        void assertConnectionStateIs(NotificationServerState s) { assert(this->_connectionState == s); };
        void assertConnectionStateIsNot(NotificationServerState s) { assert(this->_connectionState != s); };
        void assertConnectionStateIsAtLeast(NotificationServerState s) { assert(this->_connectionState >= s); };        
private:
        std::vector<SwitchboardServerConnection *> _switchboardConnections;
        std::vector<Soap *> _SoapConnections;
        std::map<int, std::pair<NotificationServerCallback, void *> > callbacks;

        ListSyncInfo *listInfo;

        std::vector<MSN::Soap::sitesToAuth> sitesToAuthList;
        std::vector<MSN::Soap::OIM> SentQueuedOIMs;
        std::vector<std::string> DeletedQueuedOIMs;

        std::string lockkey;
        bool generatingLockkey;
        bool removingOIM;

        void sendQueuedOIMs();

        // mdi value got by tweener
        std::string mdi;

        virtual void disconnectForTransfer();        
            
        static std::map<std::string, void (NotificationServerConnection::*)(std::vector<std::string> &)> commandHandlers;
        static std::map<std::string, void (NotificationServerConnection::*)(std::vector<std::string> &, std::string, std::string)> messageHandlers;

        void registerHandlers();
        void handle_OUT(std::vector<std::string> & args);
        void handle_RML(std::vector<std::string> & args);
        void handle_BLP(std::vector<std::string> & args);
        void handle_CHG(std::vector<std::string> & args);
        void handle_CHL(std::vector<std::string> & args);
        void handle_ILN(std::vector<std::string> & args);
        void handle_NLN(std::vector<std::string> & args);
        void handle_FLN(std::vector<std::string> & args);
        void handle_MSG(std::vector<std::string> & args);
        void handle_RNG(std::vector<std::string> & args);
        void handle_PRP(std::vector<std::string> & args);
        void handle_UBX(std::vector<std::string> & args);
        void handle_GCF(std::vector<std::string> & args);
        void handle_ADL(std::vector<std::string> & args);
        void handle_UBN(std::vector<std::string> & args);
        void handle_FQY(std::vector<std::string> & args);
        
        void callback_NegotiateCVR(std::vector<std::string> & args, int trid, void *data);
        void callback_TransferToSwitchboard(std::vector<std::string> & args, int trid, void *data);
        void callback_RequestUSR(std::vector<std::string> & args, int trid, void *data);
        void callback_PassportAuthentication(std::vector<std::string> & args, int trid, void * data);        
        void callback_AuthenticationComplete(std::vector<std::string> & args, int trid, void * data);
        void callback_initialBPL(std::vector<std::string> & args, int trid, void *data);
        void callback_URL(std::vector<std::string> & args, int trid, void *data);
    

        void message_initial_email_notification(std::vector<std::string> & args, std::string mime, std::string body);
        void message_email_notification(std::vector<std::string> & args, std::string mime, std::string body);
        void message_msmsgsprofile(std::vector<std::string> & args, std::string mime, std::string body);
        void message_initialmdatanotification(std::vector<std::string> & args, std::string mime, std::string body);
        void message_oimnotification(std::vector<std::string> & args, std::string mime, std::string body);

        void gotMailData(std::string maildata);

    };
    
}


#endif
