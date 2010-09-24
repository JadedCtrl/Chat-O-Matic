#ifndef __msn_switchboardserver_h__
#define __msn_switchboardserver_h__

/*
 * switchboardserver.h
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

#include "message.h"
#include "authdata.h"
#include "connection.h"
#include "passport.h"
#include "p2p.h"
#include <string>
#include <cassert>

#include "libmsn_export.h"

namespace MSN
{
    class NotificationServerConnection;
    class FileTransferConnectionP2P;

    /** Represents a connection to a MSN switchboard.
     */
    class LIBMSN_EXPORT SwitchboardServerConnection : public Connection
    {
private:
        typedef void (SwitchboardServerConnection::*SwitchboardServerCallback)(std::vector<std::string> & args, int trid, void *);
        typedef void (SwitchboardServerConnection::*SwitchboardServerCallback2)(std::vector<std::string> & args, int trid, unsigned int sessionID);

        typedef struct
        {
            int chunks;
            int receivedChunks;
            std::string mime;
            std::string body;
        } MultiPacketSession;

public:
        class AuthData : public ::MSN::AuthData
        {
public:
            std::string sessionID;
            bool direct_connection;
            std::string cookie;
            const void *tag;
            
            AuthData(Passport & username_, const std::string & sessionID_, 
                     const std::string & cookie_, const void *tag_=NULL) : 
                ::MSN::AuthData(username_), sessionID(sessionID_), cookie(cookie_), tag(tag_) {};
            
            AuthData(Passport & username_, const void *tag_=NULL) :
                ::MSN::AuthData(username_), sessionID(""), cookie(""), tag(tag_) {};        
        };
        
        SwitchboardServerConnection::AuthData auth;
        
        /** A list of the users in this switchboard session.
         */
        std::list<Passport> users;

        P2P p2p;

        SwitchboardServerConnection(AuthData & auth_, NotificationServerConnection &);
        virtual ~SwitchboardServerConnection();
        virtual void dispatchCommand(std::vector<std::string> & args);
        
        /** Return a connection that is associated with @a fd.
         *
         *  If @a fd is equal to @p sock, @c this is returned.  Otherwise
         *  connectionWithSocket is sent to each FileTransferConnection
         *  until a match is found.
         *  
         *  @return  The matching connection, if found.  Otherwise, @c NULL.
         */        
        Connection *connectionWithSocket(void *sock);
        
        std::map<std::string, MultiPacketSession> MultiPacketSessions;

        /** Add a FileTransferConnection to the list of associated connections.
         */
        void addFileTransferConnectionP2P(FileTransferConnectionP2P *);

        /** Remove a FileTransferConnection from the list of associated connections.
         */
        void removeFileTransferConnectionP2P(FileTransferConnectionP2P *);

        /** Send a typing notification to the switchboard server.
         */
        void sendTypingNotification();

        /** Send a text action
         */
        void sendAction(std::string action);

        /** Send a Voice Clip
         */
        void sendVoiceClip(std::string msnobject);

        /** Send a Wink
         */
        void sendWink(std::string msnobject);

        /** Send Ink
         */
        void sendInk(std::string image);

        /** Send an emoticon
         */
        void sendEmoticon(std::string alias, std::string file);

        /** Send a nudge
         */
        void sendNudge();
        
        /** Send a keep alive message
         */
        void sendKeepAlive();

        /** Send a file
         */
        void sendFile(MSN::fileTransferInvite ft);

        /** Invite @a userName into this conversation.
         */
        void inviteUser(Passport userName);

        /** Response to a file transfer invitation
         */
        void fileTransferResponse(unsigned int sessionID, std::string filename, bool response);

        /** Cancel a file transfer in progress
         */
        void cancelFileTransfer(unsigned int sessionID);

        virtual void connect(const std::string & hostname, unsigned int port);
        virtual void disconnect();

        /** Send formatted message, returns the trID
         */
        virtual int sendMessage(const Message *msg);

        /** Send plain text message, returns the trID
         */
        virtual int sendMessage(const std::string & s);

        /** Add @a cb as a callback that will be called when a response is received
         *  a transaction ID of @a trid.
         */
        virtual void addCallback(SwitchboardServerCallback, int trid, void *data);

        // callback of msg acks
        virtual void addP2PCallback(SwitchboardServerCallback2, int trid, unsigned int sessionID);

        /** Remove callbacks for transaction ID @a trid.
         */
        virtual void removeCallback(int trid);

        virtual void removeP2PCallback(int trid);
        
        virtual void socketConnectionCompleted();
        
        enum SwitchboardServerState
        {
            SB_DISCONNECTED,
            SB_CONNECTING,
            SB_CONNECTED,            
            SB_WAITING_FOR_USERS,
            SB_READY
        };
        
        SwitchboardServerState connectionState() const { return this->_connectionState; };
        virtual NotificationServerConnection *myNotificationServer() { return &notificationServer; };
        void callback_continueTransfer(std::vector<std::string> & args, int trid, unsigned int sessionID);

        /** Request an emoticon
         */
        void requestEmoticon(unsigned int id, std::string filename, std::string msnobject, std::string alias);

        /** Request a voice clip
         */
        void requestVoiceClip(unsigned int id, std::string filename, std::string msnobject);

        /** Request a wink 
         */
        void requestWink(unsigned int id, std::string filename, std::string msnobject);

        /** Request a display picture
         */
        void requestDisplayPicture(unsigned int id, std::string filename, std::string msnobject);
protected:
        virtual void handleIncomingData();
        SwitchboardServerState _connectionState;

        void setConnectionState(SwitchboardServerState s) { this->_connectionState = s; };
        void assertConnectionStateIs(SwitchboardServerState s) { assert(this->_connectionState == s); };
        void assertConnectionStateIsNot(SwitchboardServerState s) { assert(this->_connectionState != s); };
        void assertConnectionStateIsAtLeast(SwitchboardServerState s) { assert(this->_connectionState >= s); };
private:
        NotificationServerConnection & notificationServer;
        std::list<FileTransferConnectionP2P *> _fileTransferConnectionsP2P;

        std::map<int, std::pair<SwitchboardServerCallback, void *> > callbacks;
        std::map<int, std::pair<SwitchboardServerCallback2, unsigned int> > callbacks2;
        
        static std::map<std::string, void (SwitchboardServerConnection::*)(std::vector<std::string> &)> commandHandlers;
        static std::map<std::string, void (SwitchboardServerConnection::*)(std::vector<std::string> &, std::string, std::string)> messageHandlers;
        void registerCommandHandlers(); 
        void registerMessageHandlers(); 
        void handle_BYE(std::vector<std::string> & args);
        void handle_JOI(std::vector<std::string> & args);
        void handle_NAK(std::vector<std::string> & args);
        void handle_MSG(std::vector<std::string> & args);
        
        void callback_InviteUsers(std::vector<std::string> & args, int trid, void * data);
        void callback_AnsweredCall(std::vector<std::string> & args, int trid, void * data);
        void callback_messageACK(std::vector<std::string> & args, int trid, void * data);
        
        void handleInvite(Passport from, const std::string & friendly, const std::string & mime, const std::string & body);
        void handleNewInvite(Passport & from, const std::string & friendly, const std::string & mime, const std::string & body);
        void message_plain(std::vector<std::string> & args, std::string mime, std::string body);
        void message_invitation(std::vector<std::string> & args, std::string mime, std::string body);
        void message_typing_user(std::vector<std::string> & args, std::string mime, std::string body);
        void message_p2p(std::vector<std::string> & args, std::string mime, std::string body);
        void message_datacast(std::vector<std::string> & args, std::string mime, std::string body);
        void message_emoticon(std::vector<std::string> & args, std::string mime, std::string body);
public:
        void message_ink(std::vector<std::string> & args, std::string mime, std::string body);

        friend class Connection;
    };
}
#endif
