/*
 * msntest.cpp
 * libmsn
 *
 * Created by Meredydd Luff.
 * Refactored by Tiago Salem Herrmann
 * Copyright (c) 2004 Meredydd Luff. All rights reserved.
 * Copyright (c) 2007 Tiago Salem Herrmann. All rights reserved.
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

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/ssl.h>

#include "msn.h"
#include <string>
#include <iostream>

class Callbacks : public MSN::Callbacks
{
    
    virtual void registerSocket(void *s, int read, int write, bool isSSL);

    virtual void unregisterSocket(void *s);
    
    virtual void closeSocket(void *s);
    
    virtual void showError(MSN::Connection * conn, std::string msg);
    
    virtual void buddyChangedStatus(MSN::NotificationServerConnection * conn, MSN::Passport buddy, std::string friendlyname, MSN::BuddyStatus state, unsigned int clientID, std::string msnobject);

    virtual void buddyOffline(MSN::NotificationServerConnection * conn, MSN::Passport buddy);
    
    virtual void log(int writing, const char* buf);
   
    virtual void buddyChangedPersonalInfo(MSN::NotificationServerConnection * conn, MSN::Passport fromPassport, MSN::personalInfo);

    virtual void gotFriendlyName(MSN::NotificationServerConnection * conn, std::string friendlyname);

    virtual void gotBuddyListInfo(MSN::NotificationServerConnection * conn, MSN::ListSyncInfo * data);

    virtual void gotLatestListSerial(MSN::NotificationServerConnection * conn, std::string lastChange);

    virtual void gotGTC(MSN::NotificationServerConnection * conn, char c);

    virtual void gotBLP(MSN::NotificationServerConnection * conn, char c);
    
    virtual void addedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport buddy, std::string friendlyname);
    
    virtual void removedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport buddy);
    
    virtual void addedGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupName, std::string groupID);

    virtual void removedGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupID);
    
    virtual void renamedGroup(MSN::NotificationServerConnection * conn, bool renamed, std::string newGroupName, std::string groupID);

    virtual void addedContactToGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupId, std::string contactId);
    
    virtual void removedContactFromGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupId, std::string contactId);

    virtual void addedContactToAddressBook(MSN::NotificationServerConnection * conn, bool added, std::string passport, std::string displayName, std::string guid);

    virtual void removedContactFromAddressBook(MSN::NotificationServerConnection * conn, bool removed, std::string contactId, std::string passport);

    virtual void enabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool enabled, std::string contactId, std::string passport);
    virtual void disabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool disabled, std::string contactId);

    virtual void gotSwitchboard(MSN::SwitchboardServerConnection * conn, const void * tag);
    
    virtual void buddyJoinedConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string friendlyname, int is_initial);
    
    virtual void buddyLeftConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy);
    
    virtual void gotInstantMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string friendlyname, MSN::Message * msg);

    virtual void gotMessageSentACK(MSN::SwitchboardServerConnection * conn, int trID);

    virtual void gotEmoticonNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string alias, std::string msnobject);
    
    virtual void failedSendingMessage(MSN::Connection * conn);
    
    virtual void buddyTyping(MSN::SwitchboardServerConnection * conn, MSN::Passport buddy, std::string friendlyname);

    virtual void gotNudge(MSN::SwitchboardServerConnection * conn, MSN::Passport from);

    virtual void gotVoiceClipNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport from, std::string msnobject);
    
    virtual void gotWinkNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport from, std::string msnobject);

    virtual void gotInk(MSN::SwitchboardServerConnection * conn, MSN::Passport from, std::string image);
    
    virtual void gotVoiceClipFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file);
    
    virtual void gotEmoticonFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string alias, std::string file);

    virtual void gotWinkFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file);

    virtual void gotActionMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string message);
    
    virtual void gotInitialEmailNotification(MSN::NotificationServerConnection * conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders);
    
    virtual void gotNewEmailNotification(MSN::NotificationServerConnection * conn, std::string from, std::string subject);
    
    virtual void fileTransferProgress(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, long long unsigned transferred, long long unsigned total);
    
    virtual void fileTransferFailed(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, MSN::fileTransferError error);
    
    virtual void fileTransferSucceeded(MSN::SwitchboardServerConnection * conn, unsigned int sessionID);

    virtual void fileTransferInviteResponse(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, bool response);
    
    virtual void gotNewConnection(MSN::Connection * conn);
    
    virtual void gotOIMList(MSN::NotificationServerConnection * conn, std::vector<MSN::eachOIM> OIMs);

    virtual void gotOIM(MSN::NotificationServerConnection * conn, bool success, std::string id, std::string message);
   
    virtual void gotOIMSendConfirmation(MSN::NotificationServerConnection * conn, bool success, int id);

    virtual void gotOIMDeleteConfirmation(MSN::NotificationServerConnection * conn, bool success, std::string id);

    virtual void gotContactDisplayPicture(MSN::SwitchboardServerConnection * conn, MSN::Passport passport, std::string filename );

    virtual void closingConnection(MSN::Connection * conn);
    
    virtual void changedStatus(MSN::NotificationServerConnection * conn, MSN::BuddyStatus state);
    
    virtual void * connectToServer(std::string server, int port, bool *connected, bool isSSL);
    
    virtual void connectionReady(MSN::Connection * conn);

    virtual void askFileTransfer(MSN::SwitchboardServerConnection *conn, MSN::fileTransferInvite ft);

    virtual int listenOnPort(int port);
    
    virtual std::string getOurIP();

    virtual int getSocketFileDescriptor (void *sock);

    virtual size_t getDataFromSocket (void *sock, char *data, size_t size);

    virtual size_t writeDataToSocket (void *sock, char *data, size_t size);

    virtual std::string getSecureHTTPProxy();
    virtual void gotInboxUrl (MSN::NotificationServerConnection *conn, MSN::hotmailInfo info);
};

struct pollfd mySockets[21];
struct ssl {
    bool isSSL;
    bool isConnected;
    SSL *ssl;
    SSL_CTX *ctx;
} mySocketsSsl[21];

void handle_command(MSN::NotificationServerConnection &);
int countsocks(void);
std::string myFriendlyName;
std::string myUsername;
//std::string lastObject;

// should be random
unsigned int sessionID = 123456;

int main()
{
    for (int i = 1; i < 20; i++)
    {
        mySockets[i].fd = -1;
        mySockets[i].events = POLLIN;
        mySockets[i].revents = 0;
        mySocketsSsl[i].isSSL = false;
        mySocketsSsl[i].isConnected = false;
        mySocketsSsl[i].ctx = NULL;
        mySocketsSsl[i].ssl = NULL;
    }

    mySockets[0].fd = 0;
    mySockets[0].events = POLLIN;
    mySockets[0].revents = 0;
    
    Callbacks cb;
    MSN::Passport uname;
    char *pass = NULL;
    while (1)
    {
        fprintf(stderr, "Enter your login name: ");
        fflush(stdout);
        try
        {
            std::cin >> uname;
            myUsername = uname;
            break;
        }
        catch (MSN::InvalidPassport & e)
        {
            std::cout << e.what() << std::endl;
        }
    }
    
    pass = getpass("Enter your password: ");
    fprintf(stderr, "Connecting to the MSN Messenger service...\n");
    
    MSN::NotificationServerConnection mainConnection(uname, pass, cb);
    mainConnection.connect("messenger.hotmail.com", 1863);
    fprintf(stderr, "> ");
    fflush(stderr);
    while (1)
    {
        poll(mySockets, 20, -1);
        for (int i = 1; i < 20; i++)
        {
            if (mySockets[i].fd == -1)
                break; 
            if (mySockets[i].revents & POLLHUP) {
                mySockets[i].revents = 0; 
                continue; 
            }
            if (mySockets[i].revents & (POLLIN | POLLOUT | POLLPRI))
            {
                MSN::Connection *c;
                
                // Retrieve the connection associated with the
                // socket's file handle on which the event has
                // occurred.
                c = mainConnection.connectionWithSocket((void*)mySockets[i].fd);
                
                // if this is a libmsn socket
                if (c != NULL)
                {
                    // If we aren't connected yet, a socket event means that
                    // our connection attempt has completed.
                    if(mySocketsSsl[i].isSSL && !mySocketsSsl[i].isConnected)
                    {
                        BIO *bio_socket_new;
                        SSL_METHOD *meth=NULL;
                        meth=const_cast<SSL_METHOD *>(SSLv23_client_method());
                        SSLeay_add_ssl_algorithms();
                        mySocketsSsl[i].ctx = SSL_CTX_new(meth);
                        mySocketsSsl[i].ssl = SSL_new(mySocketsSsl[i].ctx);
                        bio_socket_new = BIO_new_socket(mySockets[i].fd, BIO_CLOSE);
                        if(!mySocketsSsl[i].ssl)
                            break;
                        BIO_set_nbio(bio_socket_new, 0);
                        SSL_set_bio(mySocketsSsl[i].ssl, bio_socket_new, bio_socket_new);
                        SSL_set_mode(mySocketsSsl[i].ssl, SSL_MODE_AUTO_RETRY);

                        // TODO - fix-me - not async and buggy
                        int ret = SSL_connect(mySocketsSsl[i].ssl);
                        mySocketsSsl[i].isConnected = true;
                    }
                    if (c->isConnected() == false)
                        c->socketConnectionCompleted();

                    // If this event is due to new data becoming available 
                    if (mySockets[i].revents & POLLIN)
                    {
                        if(mySocketsSsl[i].isSSL && mySocketsSsl[i].isConnected)
                        {
                             if(SSL_want_read(mySocketsSsl[i].ssl))
                             {
                                 mySockets[i].revents = 0;
                                 continue;
                             }
                        }
                        c->dataArrivedOnSocket();
                    }
                   
                    // If this event is due to the socket becoming writable
                    if (mySockets[i].revents & POLLOUT)
                    {
                        c->socketIsWritable();
                    }
                }
            }

            if (mySockets[i].revents & (POLLERR | POLLNVAL))
            {
                printf("Dud socket (%d)! Code %x (ERR=%x, INVAL=%x)\n", mySockets[i].fd, mySockets[i].revents, POLLERR, POLLNVAL);
                
                MSN::Connection *c;
                
                // Retrieve the connection associated with the
                // socket's file handle on which the event has
                // occurred.
                c = mainConnection.connectionWithSocket((void*)mySockets[i].fd);
 
                // if this is a libmsn socket
                if (c != NULL)
                {
                    // Delete the connection.  This will cause the resources
                    // that are being used to be freed.
                    delete c;
                }
                
                mySockets[i].fd = -1;
                mySockets[i].revents = 0;
                continue;
            }
        }

        if (mySockets[0].revents & POLLIN)
        {
            handle_command(mainConnection);
            mySockets[0].revents = 0;
        }
    }
}

void handle_command(MSN::NotificationServerConnection & mainConnection)
{
    char command[40];
    
    if (scanf(" %s", command) == EOF)
    {
        printf("\n");
        exit(0);
    }
    
    if (!strcmp(command, "quit"))
    {
        exit(0);
    } else if (!strcmp(command, "sendoim")) {
        char rcpt[80];
        char msg[1024];
        
        scanf(" %s", rcpt);
        
        fgets(msg, 1024, stdin);
        
        msg[strlen(msg)-1] = '\0';
        
        const std::string rcpt_ = rcpt;
        const std::string msg_ = msg;

        MSN::Soap::OIM oim;
        oim.myFname = myFriendlyName;
        oim.toUsername = rcpt;
        oim.message = msg;
        oim.myUsername = myUsername; 
        oim.id = 1; 

        mainConnection.send_oim(oim);
    } else if (!strcmp(command, "addtogroup")) {
        char gid[80];
        char uid[80];
        
        scanf(" %s", gid);
        
        fgets(uid, 80, stdin);
        
        uid[strlen(uid)-1] = '\0';
        
        const std::string uid_ = uid;
        const std::string gid_ = gid;

        mainConnection.addToGroup(gid,uid);
    } else if (!strcmp(command, "delfromgroup")) {
        char gid[80];
        char uid[80];
        
        scanf(" %s", gid);
        
        fgets(uid, 80, stdin);
        
        uid[strlen(uid)-1] = '\0';
        
        const std::string uid_ = uid;
        const std::string gid_ = gid;

        mainConnection.removeFromGroup(gid,uid);
    } else if (!strcmp(command, "renamegroup")) {
        char gid[80];
        char name[80];
        
        scanf(" %s", gid);
        
        fgets(name, 80, stdin);
        
        name[strlen(name)-1] = '\0';
        
        const std::string name_ = name;
        const std::string gid_ = gid;

        mainConnection.renameGroup(gid,name);
    } else if (!strcmp(command, "getoim")) {
        char id[80];
        
        scanf(" %s", id);
        
        const std::string _id = id;
        mainConnection.get_oim(id,false);
    } else if (!strcmp(command, "addgroup")) {
        char gname[80];
        
        scanf(" %s", gname);
        
        const std::string _gname = gname;
        mainConnection.addGroup(_gname);
    } else if (!strcmp(command, "delgroup")) {
        char gname[80];
        
        scanf(" %s", gname);
        
        const std::string _gname = gname;
        mainConnection.removeGroup(_gname);
    } else if (!strcmp(command, "deloim")) {
        char id[80];
        
        scanf(" %s", id);
        
        const std::string _id = id;
        mainConnection.delete_oim(id);

    } else if (!strcmp(command, "msg")) {
        char rcpt[80];
        char msg[1024];
        
        scanf(" %s", rcpt);
        
        fgets(msg, 1024, stdin);
        
        msg[strlen(msg)-1] = '\0';
        
        const std::string rcpt_ = rcpt;
        const std::string msg_ = msg;
        const std::pair<std::string, std::string> *ctx = new std::pair<std::string, std::string>(rcpt_, msg_);
        mainConnection.requestSwitchboardConnection(ctx);
    } else if (!strcmp(command, "status")) {
        char state[10];
        
        scanf(" %s", state);
        mainConnection.change_DisplayPicture("/tmp/global-photo.png");
        uint clientid=0;

        clientid += MSN::MSNC7;
        clientid += MSN::MSNC6;
        clientid += MSN::MSNC5;
        clientid += MSN::MSNC4;
        clientid += MSN::MSNC3;
        clientid += MSN::MSNC2;
        clientid += MSN::MSNC1;
        clientid += MSN::SupportWinks;
        clientid += MSN::VoiceClips;
        clientid += MSN::InkGifSupport;
        clientid += MSN::SIPInvitations;
        clientid += MSN::SupportMultiPacketMessaging;

        mainConnection.setState(MSN::buddyStatusFromString(state), clientid);
    } else if (!strcmp(command, "friendlyname")) {
        char fn[256];
        
        fgets(fn, 256, stdin);

        fn[strlen(fn)-1] = '\0';
        
        mainConnection.setFriendlyName(fn);
    } else if (!strcmp(command, "addtolist")) {
        MSN::ContactList list;
        char user[128];
        
        scanf(" %d %s", (int*)&list, user);
        
        mainConnection.addToList(list, user);
    } else if (!strcmp(command, "add")) {
        char user[128];
        char nick[128];
        
        scanf(" %s %s", user, nick);
        
        mainConnection.addToAddressBook(user, nick);
    } else if (!strcmp(command, "block")) {
        char user[128];
        
        scanf(" %s", user);
        
        mainConnection.blockContact(user);
    } else if (!strcmp(command, "unblock")) {
        char user[128];
        
        scanf(" %s", user);
        
        mainConnection.unblockContact(user);
    } else if (!strcmp(command, "delfromlist")) {
        MSN::ContactList list;
        char user[128];
        
        scanf(" %d %s", (int*)&list, user);
        
        mainConnection.removeFromList(list, user);
    } else if (!strcmp(command, "del")) {
        char contactid[128];
        char passport[128];
        
        scanf(" %s %s", contactid, passport);
        
        mainConnection.delFromAddressBook(contactid, passport);
    } else if (!strcmp(command, "enable")) {
        char contactid[128];
        char passport[128];
        
        scanf(" %s %s", contactid, passport);
        
        mainConnection.enableContactOnAddressBook(contactid,passport);
    } else if (!strcmp(command, "disable")) {
        char contactid[128];
        char passport[128];
        
        scanf(" %s %s", contactid, passport);
        
        mainConnection.disableContactOnAddressBook(contactid,passport);
    } else if (!strcmp(command, "reconnect")) {
        if (mainConnection.connectionState() != MSN::NotificationServerConnection::NS_DISCONNECTED)
            mainConnection.disconnect();

        mainConnection.connect("messenger.hotmail.com", 1863);
    } else if (!strcmp(command, "disconnect")) {
        mainConnection.disconnectNS();
    } else if (!strcmp(command, "inboxurl")) {
        mainConnection.getInboxUrl();
    } else {
        fprintf(stderr, "\nBad command \"%s\"", command);
    }
    
    fprintf(stderr, "\n> ");
    fflush(stderr);
}

int countsocks(void)
{
    int retval = 0;
    
    for (int a = 0; a < 20; a++)
    {
        if (mySockets[a].fd == -1) 
            break; 
        
        retval++;
    }
    return retval;
}

void Callbacks::registerSocket(void *s, int reading, int writing, bool isSSL)
{
    for (int a = 1; a < 20; a++)
    {
        if (mySockets[a].fd == -1 || mySockets[a].fd == getSocketFileDescriptor(s))
        {
            if(mySockets[a].fd == -1)
                mySockets[a].events = 0;
            if (reading)
                mySockets[a].events |= POLLIN;
 
            if (writing)
                mySockets[a].events |= POLLOUT;

            mySockets[a].fd = getSocketFileDescriptor(s);
            if(isSSL)
                mySocketsSsl[a].isSSL = true;
            return;
        }
    }
}

int
Callbacks::getSocketFileDescriptor (void *sock)
{
    long a = (long)sock;
    return (int)a;
}

void Callbacks::closeSocket(void *s)
{
    for (int a = 1; a < 20; a++)
    {
        if (mySockets[a].fd == getSocketFileDescriptor(s))
        {
            close(getSocketFileDescriptor(s));
            if(mySocketsSsl[a].isSSL)
            {
                if(mySocketsSsl[a].ssl)
                {
                    if(mySocketsSsl[a].ssl) SSL_set_shutdown(mySocketsSsl[a].ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
                    if(mySocketsSsl[a].ssl) SSL_free(mySocketsSsl[a].ssl);
                    if(mySocketsSsl[a].ctx) SSL_CTX_free(mySocketsSsl[a].ctx);
                    mySocketsSsl[a].ssl=NULL;
                    mySocketsSsl[a].ctx=NULL;
                }
            }
            for (int b = a; b < 19; b++)
            {
                mySockets[b].fd = mySockets[b + 1].fd;
                mySockets[b].revents = mySockets[b + 1].revents;
                mySockets[b].events = mySockets[b + 1].events;
                mySocketsSsl[b].isSSL = mySocketsSsl[b + 1].isSSL;
                mySocketsSsl[b].isConnected = mySocketsSsl[b + 1].isConnected;
                mySocketsSsl[b].ssl = mySocketsSsl[b + 1].ssl;
                mySocketsSsl[b].ctx = mySocketsSsl[b + 1].ctx;
            }
            mySockets[19].fd = -1;
            mySocketsSsl[19].isConnected = false;
            mySocketsSsl[19].isSSL = false;
            mySocketsSsl[19].ssl = NULL;
            mySocketsSsl[19].ctx = NULL;
        }
    }
}

void Callbacks::unregisterSocket(void *s)
{
    for (int a = 1; a < 20; a++)
    {
        if (mySockets[a].fd == getSocketFileDescriptor(s))
        {
            mySockets[a].events = 0;
        }
    }
}

void Callbacks::gotFriendlyName(MSN::NotificationServerConnection * conn, std::string friendlyname)
{
    myFriendlyName = friendlyname.c_str();
    printf("Your friendlyname is now: %s\n", friendlyname.c_str());
}

void Callbacks::gotBuddyListInfo(MSN::NotificationServerConnection * conn, MSN::ListSyncInfo * info)
{
    // IMPORTANT
    // Here you need to fill a vector with all your contacts
    // both received by the server and previous ones.
    // Next pass this vector to the function completeConnection()
    // if you dont call completeConnection(), the service will
    // not work.
    std::map<std::string, MSN::Buddy *>::iterator i = info->contactList.begin();
    std::map<std::string, int> allContacts;
        
    for (; i != info->contactList.end(); i++)
    {
        MSN::Buddy *contact = (*i).second;
        if(contact->lists & MSN::LST_AB ) // only if it is the address book
        {
            allContacts[contact->userName.c_str()]=0;
            allContacts[contact->userName.c_str()] |= MSN::LST_AB;
            printf("-AB %s (%s)\n    Id: %s\n", contact->friendlyName.c_str(), contact->userName.c_str(), contact->properties["contactId"].c_str());

            if(contact->properties["isMessengerUser"]=="false")
                printf("    Not Messenger User\n");

            std::list<MSN::Buddy::PhoneNumber>::iterator pns = contact->phoneNumbers.begin();
            std::list<MSN::Group *>::iterator g = contact->groups.begin();

            for (; g != contact->groups.end(); g++)
            {
                printf("    G: %s\n", (*g)->name.c_str());
            }
    
            for (; pns != contact->phoneNumbers.end(); pns++)
            {
                printf("    %s: %s (%d)\n", (*pns).title.c_str(), (*pns).number.c_str(), (*pns).enabled);
            }
        }
        if(contact->lists & MSN::LST_AL )
        {
            allContacts[contact->userName.c_str()] |= MSN::LST_AL;
            printf("-AL %s \n", contact->userName.c_str());
        }

        if(contact->lists & MSN::LST_BL )
        {
            allContacts[contact->userName.c_str()] |= MSN::LST_BL;
            printf("-BL %s \n", contact->userName.c_str());
        }

        if(contact->lists & MSN::LST_RL )
        {
            printf("-RL %s \n", contact->userName.c_str());
        }
        if(contact->lists & MSN::LST_PL )
        {
            printf("-PL %s \n", contact->userName.c_str());
        }
    }
    printf("Available Groups:\n");
    std::map<std::string, MSN::Group>::iterator g = info->groups.begin();

    for (; g != info->groups.end(); g++)
    {
        printf("    %s: %s\n", (*g).second.groupID.c_str(), (*g).second.name.c_str());
    }

    std::map<std::string, int>::iterator b = allContacts.begin();

    // this will send the ADL command to the server
    // It is necessary. Dont forget to add *all* your contacts to allContacts,
    // (both Forward, allow and block lists) or you probably will 
    // loose someone.
    // A contact cannot be present both on allow and block lists or the
    // server will return an error, so you need to let your application
    // choose the better list to put it in.
    conn->completeConnection(allContacts,info);
}

void Callbacks::gotLatestListSerial(MSN::NotificationServerConnection * conn, std::string lastChange)
{
    // The application needs to track this number to not ask for the whole contact
    // list every login
    printf("The latest change number is: %s\n", lastChange.c_str());
}

void Callbacks::gotGTC(MSN::NotificationServerConnection * conn, char c)
{
    printf("Your GTC value is now %c\n", c);
}

void Callbacks::gotOIMDeleteConfirmation(MSN::NotificationServerConnection * conn, bool success, std::string id)
{
    if(success)
        std::cout << "OIM "<< id << " removed sucessfully." << std::endl;
    else
        std::cout << "OIM "<< id << " not removed sucessfully." << std::endl;
                
}

void Callbacks::gotOIMSendConfirmation(MSN::NotificationServerConnection * conn, bool success, int id)
{
    if(success)
        std::cout << "OIM " << id << " sent sucessfully." << std::endl;
    else
        std::cout << "OIM " << id << " not sent sucessfully." << std::endl;
}

void Callbacks::gotOIM(MSN::NotificationServerConnection * conn, bool success, std::string id, std::string message)
{
    if(success)
        std::cout << "ID: " << id << std::endl << "\t" << message << std::endl;
    else
        std::cout << "Error retreiving OIM " << id << std::endl;
}

void Callbacks::gotOIMList(MSN::NotificationServerConnection * conn, std::vector<MSN::eachOIM> OIMs)
{
    if(OIMs.size()==0)
    {
        printf("No Offline messages\n");
        return;
    }
    std::vector<MSN::eachOIM>::iterator i = OIMs.begin();
    for(; i<OIMs.end();i++)
    {
        printf("Offline message from: %s\n\t - Friendly Name: %s\n\t - Id: %s\n",(*i).from.c_str(), (*i).fromFN.c_str(), (*i).id.c_str());
    }
}

void Callbacks::connectionReady(MSN::Connection * conn)
{
    printf("The connection is ready. You can change your status now!\n");
}

void Callbacks::gotBLP(MSN::NotificationServerConnection * conn, char c)
{
    printf("Your BLP value is now %cL\n", c);
}

void Callbacks::addedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport username, std::string friendlyname)
{
    if(list == MSN::LST_RL)
        // after adding the user you need to delete it from the pending list.
        // it will be added automatically by the msn service
        printf("%s is now on your list %d. FriendlyName: %s\n", username.c_str(), list, friendlyname.c_str());
    else
        // on normal lists you'll never receive the contacts displayname
        // it is not needed anyway
        printf("%s is now on your list %d\n", username.c_str(), list);
}

void Callbacks::removedListEntry(MSN::NotificationServerConnection * conn, MSN::ContactList list, MSN::Passport username)
{
    // list is a number which matches with MSN::ContactList enum on util.h
    printf("%s has been removed from list %d\n", username.c_str(), list);
}

void Callbacks::addedGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupName, std::string groupID)
{
    if(added)
        printf("A group named %s (%s) was added\n", groupName.c_str(), groupID.c_str());
    else
        printf("Group (%s) was NOT added\n", groupName.c_str());
}

void Callbacks::removedGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupID)
{
    if(removed)
        printf("A group with ID %s was removed\n", groupID.c_str());
    else
    printf("Group (%s) was NOT removed\n", groupID.c_str());
}

void Callbacks::renamedGroup(MSN::NotificationServerConnection * conn, bool renamed, std::string newGroupName, std::string groupID)
{
    if(renamed)
        printf("A group with ID %s was renamed to %s\n", groupID.c_str(), newGroupName.c_str());
    else
        printf("A group with ID %s was NOT renamed to %s\n", groupID.c_str(), newGroupName.c_str());
}

void Callbacks::showError(MSN::Connection * conn, std::string msg)
{
    printf("MSN: Error: %s\n", msg.c_str());
}

void Callbacks::buddyChangedStatus(MSN::NotificationServerConnection * conn, MSN::Passport buddy, std::string friendlyname, MSN::BuddyStatus status, unsigned int clientID, std::string msnobject)
{
    printf("%s (%s) is now %s\n", friendlyname.c_str(), buddy.c_str(), MSN::buddyStatusToString(status).c_str());
    if (clientID & MSN::SupportWebcam)
        printf("\t Supports Webcam\n");
    if (clientID & MSN::VoiceClips)
        printf("\t Supports VoiceClips\n");
    if (clientID & MSN::SupportDirectIM)
        printf("\t Supports DirectIM\n");
    if (clientID & MSN::SharingFolders)
        printf("\t Supports SharingFolders\n");
    if (clientID & MSN::MSNC1)
        printf("\t Supports MSNC1\n");
    if (clientID & MSN::MSNC2)
        printf("\t Supports MSNC2\n");
    if (clientID & MSN::MSNC3)
        printf("\t Supports MSNC3\n");
    if (clientID & MSN::MSNC4)
        printf("\t Supports MSNC4\n");
    if (clientID & MSN::MSNC5)
        printf("\t Supports MSNC5\n");
    if (clientID & MSN::MSNC6)
        printf("\t Supports MSNC6\n");
    if (clientID & MSN::MSNC7)
        printf("\t Supports MSNC7\n");
    if (clientID & MSN::InkGifSupport)
        printf("\t Supports Ink Gif\n");
    if (clientID & MSN::InkIsfSupport)
        printf("\t Supports Ink Isf\n");

//  lastObject = msnobject;
}

void Callbacks::buddyOffline(MSN::NotificationServerConnection * conn, MSN::Passport buddy)
{
    printf("%s is now offline\n", buddy.c_str());
}

void Callbacks::gotSwitchboard(MSN::SwitchboardServerConnection * conn, const void * tag)
{
    printf("Got switchboard connection\n");
    if (tag)
    {
        const std::pair<std::string, std::string> *ctx = static_cast<const std::pair<std::string, std::string> *>(tag);
        conn->inviteUser(ctx->first);
    }
}

void Callbacks::buddyJoinedConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string friendlyname, int is_initial)
{
    printf("%s (%s) is now in the session\n", friendlyname.c_str(), username.c_str());
    if (conn->auth.tag)
    {
        const std::pair<std::string, std::string> *ctx = static_cast<const std::pair<std::string, std::string> *>(conn->auth.tag);
    // Example of sending a custom emoticon
//    conn->sendEmoticon("(EMOTICON)", filename);

    int trid = conn->sendMessage(ctx->second);
    std::cout << "Message " << trid << " queued" << std::endl;
        delete ctx;
        conn->auth.tag = NULL;

        //Example of sending a file
//    MSN::fileTransferInvite ft;
//    ft.filename = "/tmp/filetosend.txt";
//    ft.friendlyname = "filetosend2.txt";
//    ft.sessionId = sessionID++;
//    ft.type = MSN::FILE_TRANSFER_WITHOUT_PREVIEW;
//    conn->sendFile(ft);

//    conn->sendNudge();
//    conn->sendAction("Action message here");

    // Exemple of requesting a display picture.
//    std::string filename2("/tmp/displayPicture.bin"+MSN::toStr(sessionID));
    // lastObject is the msnobject received on each contact status change
    // you should generate a random sessionID
//    conn->requestDisplayPicture(sessionID++, filename2, lastObject);

    // Example of sending a voice clip
//    conn->myNotificationServer()->msnobj.addMSNObject("/tmp/voiceclip.wav",11);
//    std::string obj;
//    conn->myNotificationServer()->msnobj.getMSNObjectXML("/tmp/voiceclip.wav", 11, obj);
//    conn->sendVoiceClip(obj);
    // exemple of sending an ink
//    std::string ink("base64 data here...");
//    conn->sendInk(ink);
    }
}

void Callbacks::buddyLeftConversation(MSN::SwitchboardServerConnection * conn, MSN::Passport username)
{
    printf("%s has now left the session\n", username.c_str());
}

void Callbacks::gotInstantMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string friendlyname, MSN::Message * msg)
{
    printf("--- Message from %s (%s) in font %s:\n%s\n", friendlyname.c_str(), username.c_str(), msg->getFontName().c_str(), msg->getBody().c_str());
}

void Callbacks::gotEmoticonNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string alias, std::string msnobject)
{
    std::string filename2("/tmp/emoticon.bin"+MSN::toStr(sessionID));
    printf("--- Emoticon '%s' from %s -> %s\n", alias.c_str(), username.c_str(), msnobject.c_str());
    conn->requestEmoticon(sessionID++, filename2, msnobject, alias);
}

void Callbacks::failedSendingMessage(MSN::Connection * conn)
{
    printf("**************************************************\n");
    printf("ERROR:  Your last message failed to send correctly\n");
    printf("**************************************************\n");
}

void Callbacks::buddyTyping(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string friendlyname)
{
    printf("\t%s (%s) is typing...\n", friendlyname.c_str(), username.c_str());

}

void Callbacks::gotNudge(MSN::SwitchboardServerConnection * conn, MSN::Passport username)
{
    printf("\t%s sent you a nudge ...\n", username.c_str());
}

void Callbacks::gotVoiceClipNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string msnobject)
{
    printf("\t%s sent you a voice clip...\n", username.c_str());
    std::string filename2("/tmp/voiceclip.bin"+MSN::toStr(sessionID));
    conn->requestVoiceClip(sessionID++, filename2, msnobject);
}

void Callbacks::gotWinkNotification(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string msnobject)
{
    printf("\t%s sent you a Wink...\n", username.c_str());
    std::string filename2("/tmp/wink.bin"+MSN::toStr(sessionID));
    // you should generate a random sessionID number
    conn->requestWink(sessionID++, filename2, msnobject);
}

void Callbacks::gotInk(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string image)
{
    // image variable is the base64 encoded gif file
    printf("\t%s sent you an Ink...\n", username.c_str());
//  std::string filename2("/tmp/ink.bin"+MSN::toStr(sessionID));
    // you should generate a random sessionID number
//  conn->requestFile(sessionID++, filename2, msnobject);
}

void Callbacks::gotContactDisplayPicture(MSN::SwitchboardServerConnection * conn, MSN::Passport passport, std::string filename )
{
    printf("Received display picture from %s. File: %s\n", passport.c_str(), filename.c_str());
}

void Callbacks::gotActionMessage(MSN::SwitchboardServerConnection * conn, MSN::Passport username, std::string message)
{
    printf("\t%s sent you an action message: %s\n", username.c_str(), message.c_str());
}

void Callbacks::gotInitialEmailNotification(MSN::NotificationServerConnection * conn, int msgs_inbox, int unread_inbox, int msgs_folders, int unread_folders)
{
    if (unread_inbox > 0) 
        printf("You have %d new messages in your Inbox. Total: %d\n", unread_inbox, msgs_inbox); 
    
    if (unread_folders > 0) 
        printf("You have %d new messages in other folders. Total: %d\n", unread_folders, msgs_folders); 
}

void Callbacks::gotNewEmailNotification(MSN::NotificationServerConnection * conn, std::string from, std::string subject)
{
    printf("New e-mail has arrived from %s.\nSubject: %s\n", from.c_str(), subject.c_str());
}

void Callbacks::fileTransferInviteResponse(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, bool response)
{
    if(response)
            printf("Session accepted %d: \n", sessionID);
    else
            printf("Session not accepted %d: \n", sessionID);
}

void Callbacks::fileTransferProgress(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, unsigned long long transferred, unsigned long long total)
{
    printf("File transfer: session %d\t(%llu/%llu bytes sent/received)\n", sessionID, transferred, total);
}

void Callbacks::fileTransferFailed(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, MSN::fileTransferError error)
{
    if( error == MSN::FILE_TRANSFER_ERROR_USER_CANCELED)
        printf("The other user canceled the file transfer failed. Session: %d\n", sessionID);
    if( error == MSN::FILE_TRANSFER_ERROR_UNKNOWN)
        printf("File transfer failed. Session: %d\n", sessionID);
}

void Callbacks::fileTransferSucceeded(MSN::SwitchboardServerConnection * conn, unsigned int sessionID)
{
    printf("File transfer successfully completed. session: %d\n", sessionID);
}

void Callbacks::gotNewConnection(MSN::Connection * conn)
{
    if (dynamic_cast<MSN::NotificationServerConnection *>(conn))
        dynamic_cast<MSN::NotificationServerConnection *>(conn)->synchronizeContactList();
}

void Callbacks::buddyChangedPersonalInfo(MSN::NotificationServerConnection * conn, MSN::Passport fromPassport, MSN::personalInfo pInfo) 
{
    // MSN::personalInfo shows all the data you can grab from the contact
    printf("User %s Personal Message: %s\n", fromPassport.c_str(),pInfo.PSM.c_str());
}

void Callbacks::closingConnection(MSN::Connection * conn)
{
    printf("Closed connection with socket %d\n", conn->sock);
}

void Callbacks::changedStatus(MSN::NotificationServerConnection * conn, MSN::BuddyStatus state)
{
    printf("Your state is now: %s\n", MSN::buddyStatusToString(state).c_str());

    MSN::personalInfo pInfo;
    pInfo.PSM="my personal message";
    pInfo.mediaType="Music";
    pInfo.mediaIsEnabled=1;
    pInfo.mediaFormat="{0} - {1}";
    pInfo.mediaLines.push_back("Artist");
    pInfo.mediaLines.push_back("Song");
    conn->setPersonalStatus(pInfo);
}

void * Callbacks::connectToServer(std::string hostname, int port, bool *connected, bool isSSL)
{
    struct sockaddr_in sa;
    struct hostent     *hp;
    int s;
    
    if ((hp = gethostbyname(hostname.c_str())) == NULL) {
        errno = ECONNREFUSED;                       
        return (void*)-1;
    }
    
    memset(&sa,0,sizeof(sa));
    memcpy((char *)&sa.sin_addr,hp->h_addr,hp->h_length);     /* set address */
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((u_short)port);
    
    if ((s = socket(hp->h_addrtype,SOCK_STREAM,0)) < 0)     /* get socket */
        return (void*)-1;

    int oldfdArgs = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, oldfdArgs | O_NONBLOCK);
    
    if (connect(s,(struct sockaddr *)&sa,sizeof sa) < 0)
    {
        if (errno != EINPROGRESS)
        {
            close(s);
            return (void*)-1;
        }
        *connected = false;
    }
    else
        *connected = true;
    return (void*)s;
}

int Callbacks::listenOnPort(int port)
{
    int s;
    struct sockaddr_in addr;
    
    if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        return -1;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (bind(s, (sockaddr *)(&addr), sizeof(addr)) < 0 || listen(s, 1) < 0)
    {
        close(s);
        return -1;
    }
    
    return s;
}

std::string Callbacks::getOurIP(void)
{
    struct hostent * hn;
    char buf2[1024];
    
    gethostname(buf2,1024);
    hn = gethostbyname(buf2);
    
    return inet_ntoa( *((struct in_addr*)hn->h_addr));
}

void Callbacks::log(int i, const char *s)
{
    
}

std::string Callbacks::getSecureHTTPProxy()
{
    return "";
}

void Callbacks::gotMessageSentACK(MSN::SwitchboardServerConnection * conn, int trID)
{
    std::cout << "Message " << trID << " delivered" << std::endl;
}

void Callbacks::askFileTransfer(MSN::SwitchboardServerConnection * conn, MSN::fileTransferInvite ft)
{
    std::string filename2("/tmp/"+ft.filename);
    switch(ft.type)
    {
        case MSN::FILE_TRANSFER_BACKGROUND_SHARING:
            printf("User %s wants to share with you a background file named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
            break;
        case MSN::FILE_TRANSFER_BACKGROUND_SHARING_CUSTOM:
            printf("User %s wants to share with you a *custom background file named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
            break;
        case MSN::FILE_TRANSFER_WITH_PREVIEW:
            printf("User %s wants to send you a file *with preview named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
            // ft.preview has the base64 encoded png file
            break;
        case MSN::FILE_TRANSFER_WITHOUT_PREVIEW:
            printf("User %s wants to send you a file *without preview named %s. Size: %llu. Accepting..\n", ft.userPassport.c_str(), ft.filename.c_str(), ft.filesize);
            break;
        default:
            printf("Unknown filetransfer type from %s..\n", ft.userPassport.c_str());

    }
    conn->fileTransferResponse(ft.sessionId, filename2, true);
}

void Callbacks::addedContactToGroup(MSN::NotificationServerConnection * conn, bool added, std::string groupId, std::string contactId)
{
    if(added)
        printf("User Id (%s) added to group Id (%s)\n", contactId.c_str(),groupId.c_str());
    else
        printf("User Id (%s) NOT added to group Id (%s)\n", contactId.c_str(),groupId.c_str());
}

void Callbacks::removedContactFromGroup(MSN::NotificationServerConnection * conn, bool removed, std::string groupId, std::string contactId)
{
    if(removed)
        printf("User Id (%s) removed from group Id (%s)\n", contactId.c_str(),groupId.c_str());
    else
        printf("User Id (%s) NOT removed from group Id (%s)\n", contactId.c_str(),groupId.c_str());
}

void Callbacks::addedContactToAddressBook(MSN::NotificationServerConnection * conn, bool added, std::string passport, std::string displayName, std::string guid)
{
    if(added)
        printf("User (%s - %s) added to AddressBook. Guid (%s)\n", passport.c_str(),displayName.c_str(), guid.c_str());
    else
        printf("User (%s - %s) NOT added to AddressBook.\n", passport.c_str(),displayName.c_str());
}

void Callbacks::removedContactFromAddressBook(MSN::NotificationServerConnection * conn, bool removed, std::string contactId, std::string passport)
{
    if(removed)
        printf("User %s removed from AddressBook. Guid (%s)\n", passport.c_str(), contactId.c_str());
    else
        printf("User %s NOT removed from AddressBook. Guid (%s)\n", passport.c_str(), contactId.c_str());
}

void Callbacks::enabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool enabled, std::string contactId, std::string passport)
{
    // this is used to enable a contact previously disabled from msn, but not fully removed
    if(enabled)
        printf("User (%s) enabled on AddressBook. Guid (%s)\n", passport.c_str(), contactId.c_str());
    else
        printf("User (%s) NOT enabled on AddressBook. Guid (%s)\n", passport.c_str(), contactId.c_str());
}

void Callbacks::disabledContactOnAddressBook(MSN::NotificationServerConnection * conn, bool disabled, std::string contactId)
{
    // this is used when you have disabled this user from msn, but not deleted from hotmail
    // I suggest to delete the contact instead of disable, since I haven't tested this too much yet
    if(disabled)
        printf("User disabled on AddressBook. Guid (%s)\n", contactId.c_str());
    else
        printf("User NOT disabled on AddressBook. Guid (%s)\n", contactId.c_str());
}

size_t Callbacks::getDataFromSocket (void *sock, char *data, size_t size)
{
    int fd;
    int idx=0;
    bool ssl_done = false;
    int amountRead = 0;
    int newAmountRead = 0;
    for (int i = 1; i < 20; i++)
    {
        if(mySockets[i].fd == getSocketFileDescriptor(sock))
        {
            idx = i;
            break;
        }
    }
    if(!idx)
        return 0;

    if(!mySocketsSsl[idx].isSSL)
    {
        int newWritten = ::recv(getSocketFileDescriptor(sock), data, size, 0);
        if (errno == EAGAIN)
           return -1;
        return newWritten;
    }

    if (!mySocketsSsl[idx].isConnected)
        return 0;

    while(!ssl_done)
    {
        if(!mySocketsSsl[idx].ssl) break;
        newAmountRead = SSL_read(mySocketsSsl[idx].ssl, data, size);
        switch(SSL_get_error(mySocketsSsl[idx].ssl,newAmountRead))
        {
            case SSL_ERROR_NONE:
                return newAmountRead;
            case SSL_ERROR_ZERO_RETURN:
                return 0;
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
                ssl_done=false;
                break;
            case SSL_ERROR_SSL:
                return 0;
                break;
            case SSL_ERROR_SYSCALL:
                return 0;
                break;
            default:
                return newAmountRead;
        }
    }
    return 0;
}

size_t Callbacks::writeDataToSocket (void *sock, char *data, size_t size)
{
    size_t written = 0;
    int idx;
    for (int i = 1; i < 20; i++)
    {
        if(mySockets[i].fd == getSocketFileDescriptor(sock))
        {
            idx = i;
            break;
        }
    }

    while (written < size)
    {
        int newWritten;
        if (mySocketsSsl[idx].isSSL)
        {
            if(!mySocketsSsl[idx].isConnected)
                return 0;
            newWritten = SSL_write(mySocketsSsl[idx].ssl, data, (int) (size - written));
            int error = SSL_get_error(mySocketsSsl[idx].ssl,newWritten);
            switch(error)
            {
                case SSL_ERROR_NONE:
                    written += newWritten;
                    data+=newWritten;
                    break;
                case SSL_ERROR_WANT_WRITE:
                case SSL_ERROR_WANT_READ:
                    continue;
                case SSL_ERROR_ZERO_RETURN:
                    break;
                case SSL_ERROR_SYSCALL:
                default:
                    break;
            }
        }
        else
        {
            newWritten = ::send(getSocketFileDescriptor(sock), data, (int)(size - written), 0);

            if (newWritten <= 0)
            {
                if (errno == EAGAIN)
                    continue;
                else
                    break;
            }
            written += newWritten;
            data+=newWritten;
        }
    }
    if (written != size)
    {
        // TODO: return an error
        showError(NULL, "Error on socket");
        unregisterSocket(sock);
        closeSocket(sock);
        return written;
    }
    return written;
}

void Callbacks::gotVoiceClipFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file)
{
    printf("--- Voice clip file '%s'\n", file.c_str());

}

void Callbacks::gotEmoticonFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string alias, std::string file)
{
    printf("--- Voice emoticon file '%s' for '%s'\n", file.c_str(), alias.c_str());
}

void Callbacks::gotWinkFile(MSN::SwitchboardServerConnection * conn, unsigned int sessionID, std::string file)
{
    printf("--- Voice Wink file '%s'\n", file.c_str());
}

void Callbacks::gotInboxUrl(MSN::NotificationServerConnection *conn, MSN::hotmailInfo info)
{
    std::vector<std::string>::const_iterator i;

    std::cout << "--- Inbox URL data :" << std::endl;
    std::cout << "sid: " << info.sid << std::endl;
    std::cout << "kv: " << info.kv << std::endl;
    std::cout << "id: " << info.id << std::endl;
    std::cout << "sl: " << info.sl << std::endl;
    std::cout << "rru: " << info.rru << std::endl;
    std::cout << "auth: " << info.MSPAuth << std::endl;
    std::cout << "creds: " << info.creds << std::endl;
}
