#ifndef __msn_soap_h__
#define __msn_soap_h__
/*
 * soap.h
 * libmsn
 *
 * Crated by Tiago Salem Herrmann on 08/2007.
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


#include "connection.h"
#include "authdata.h"
#include "errorcodes.h"
#include "buddy.h"
#include "passport.h"
#include <stdexcept>
#include "externals.h"

#include <iostream>
#include <vector>
#include <map>

#include "libmsn_export.h"

namespace MSN
{
    class NotificationServerConnection;

    /** Represents a Soap Connection made by NotificationServerConnection
     */
    class LIBMSN_EXPORT Soap : public Connection
    {
private:
        NotificationServerConnection & notificationServer;
        std::string request_body;
        std::string http_header_response;
        std::string response_body;
        int action;
        unsigned int response_length;
        ListSyncInfo *listInfo;
        std::string oim_id;
        std::string http_response_code;
        std::string tempDisplayName;
        std::string tempPassport;
        std::string contactId;
        std::string groupId;
        std::string groupName;
        MSN::ContactList tempList;
        std::string passport;
        std::string password;
        std::string policy;
        std::string mbi;
        std::string myDisplayName;
        bool markAsRead;

public:
        struct sitesToAuthTAG
        {
            std::string url;
            std::string URI;
            std::string BinarySecurityToken;
            std::string BinarySecret;
        };
    
        typedef sitesToAuthTAG sitesToAuth;

        struct OIMTAG 
        {
            int id;
            std::string toUsername;
            std::string myUsername;
            std::string myFname;
            std::string message;
            std::string full_msg; // includes b64 body
        };
    
        typedef OIMTAG OIM;

        MSN::Soap::OIM oim;
        Soap(NotificationServerConnection & _myNotificationServer);
        Soap(NotificationServerConnection & _myNotificationServer, std::vector<sitesToAuth> sitesToAuthList);
        ~Soap();
        std::string body;
        std::string ticket_token;
        std::string lockkey;

        enum memberRoles { // the lists
            ALLOW_LIST = 2,
            BLOCK_LIST = 4,
            REVERSE_LIST = 8,
            PENDING_LIST = 16
        };

        typedef enum {
            AUTH,
            GET_LISTS,
            GET_ADDRESS_BOOK,
            ADD_CONTACT_TO_LIST,
            DEL_CONTACT_FROM_LIST,
            ADD_CONTACT_TO_ADDRESSBOOK,
            DEL_CONTACT_FROM_ADDRESSBOOK,
            ENABLE_CONTACT_ON_ADDRESSBOOK,
            DISABLE_CONTACT_ON_ADDRESSBOOK,
            ADD_GROUP,
            DEL_GROUP,
            RENAME_GROUP,
            BLOCK_CONTACT,
            UNBLOCK_CONTACT,
            ADD_CONTACT_TO_GROUP,
            DEL_CONTACT_FROM_GROUP,
            UPDATE_GROUP,
            GENERATE_LOCKKEY,
            RETRIEVE_OIM_MAIL_DATA,
            RETRIEVE_OIM,
            DELETE_OIM,
            SEND_OIM,
            CHANGE_DISPLAYNAME
        } soapAction;

        static std::map<int,std::string> actionDomains;
        static std::map<int,std::string> actionPOSTURLs;
        static std::map<int,std::string> actionURLs;
        std::vector<sitesToAuth> sitesToAuthList;

        void fillURLs();
        void setMBI(std::string MBI);
        void requestSoapAction(soapAction action, std::string xml_body, std::string & xml_response);

        void getTickets(std::string Passport, 
                     std::string password, 
                     std::string policy);
        void parseGetTicketsResponse(std::string response);

        void getLists(ListSyncInfo* data);
        void parseGetListsResponse(std::string response);

        void getAddressBook(ListSyncInfo *info);
        void parseGetAddressBookResponse(std::string response);

        void getOIM(std::string id, bool markAsRead);
        void parseGetOIMResponse(std::string response);

        void deleteOIM(std::string id);
        void parseDeleteOIMResponse(std::string response);

        void getMailData();
        void parseGetMailDataResponse(std::string response);

        void sendOIM(OIM oim, std::string lockkey);
        void parseSendOIMResponse(std::string response);

        void addContactToList(MSN::Passport passport, MSN::ContactList list);
        void parseAddContactToListResponse(std::string response);

        void addContactToAddressBook(std::string passport, std::string displayName);
        void parseAddContactToAddressBookResponse(std::string response);

        void delContactFromAddressBook(std::string contactId, std::string passport);
        void parseDelContactFromAddressBookResponse(std::string response);

        void enableContactOnAddressBook(std::string contactId, 
                std::string passport, 
                std::string myDisplayName);

        void parseEnableContactOnAddressBookResponse(std::string response);

        void disableContactFromAddressBook(std::string contactId, std::string passport);
        void parseDisableContactFromAddressBookResponse(std::string response);

        void addContactToGroup(std::string groupId, std::string contactId);
        void parseAddContactToGroupResponse(std::string response);

        void delContactFromGroup(std::string groupId, std::string contactId);
        void parseDelContactFromGroupResponse(std::string response);

        void removeContactFromList(MSN::Passport passport, MSN::ContactList list);
        void parseRemoveContactFromListResponse(std::string response);

        void addGroup(std::string groupName);
        void parseAddGroupResponse(std::string response);

        void delGroup(std::string groupId);
        void parseDelGroupResponse(std::string response);

        void renameGroup(std::string groupId, std::string newGroupName);
        void parseRenameGroupResponse(std::string response);

        void generateLockkey(OIM oim);
        void parseGenerateLockkeyResponse(std::string response);

        void changeDisplayName(std::string newDisplayName);
        void parseChangeDisplayNameResponse(std::string);

        virtual void dispatchCommand(std::vector<std::string> &) {};
        virtual void connect(const std::string &, unsigned int) {};
        virtual void disconnect();
        virtual void sendMessage(const Message *) {};
        virtual void sendMessage(const std::string &) {};
        virtual void socketConnectionCompleted();
        virtual void handleIncomingData();
        virtual NotificationServerConnection *myNotificationServer() { return &notificationServer; };

    };

}
#endif
