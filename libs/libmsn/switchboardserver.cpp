/*
 * switchboardserver.cpp
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

#include "switchboardserver.h"
#include "notificationserver.h"
#include "errorcodes.h"
#include "externals.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <algorithm>
#include <cassert>
#include <sstream>

namespace MSN
{
    std::map<std::string, void (SwitchboardServerConnection::*)(std::vector<std::string> &)> SwitchboardServerConnection::commandHandlers;
    std::map<std::string, void (SwitchboardServerConnection::*)(std::vector<std::string> &, std::string, std::string)> SwitchboardServerConnection::messageHandlers;
    
    SwitchboardServerConnection::SwitchboardServerConnection(AuthData & auth_, NotificationServerConnection & n)
        : Connection(), auth(auth_), _connectionState(SB_DISCONNECTED), notificationServer(n)
    {
        registerCommandHandlers();
        registerMessageHandlers();
    }
    
    SwitchboardServerConnection::~SwitchboardServerConnection()
    {
        if (this->connectionState() != SB_DISCONNECTED)
            this->disconnect();
    }
     
    Connection *SwitchboardServerConnection::connectionWithSocket(void * sock)
    {
        if (this->sock == sock)
            return this;

/*  std::list<FileTransferConnectionP2P *> & list2 = _fileTransferConnectionsP2P;
        std::list<FileTransferConnectionP2P *>::iterator j = list2.begin();
        
        for (; j != list2.end(); j++)
        {
            if ((*j)->sock == fd)
                return *i;
        }
    */
        return NULL;
    }

    void SwitchboardServerConnection::sendEmoticon(std::string alias, std::string file)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);

        myNotificationServer()->msnobj.addMSNObject(file,2);
        std::string msnobject;
        myNotificationServer()->msnobj.getMSNObjectXML(file, 2, msnobject);

        std::ostringstream buf_, msg_;
        msg_ << "MIME-Version: 1.0\r\n";
        msg_ << "Content-Type: text/x-mms-emoticon\r\n\r\n";
        msg_ << alias << "\t" << msnobject << "\t";
        size_t msg_length = msg_.str().size();

        buf_ << "MSG " << this->trID++ << " N " << (int) msg_length << "\r\n" << msg_.str();
        write(buf_);
    }

    void SwitchboardServerConnection::sendFile(MSN::fileTransferInvite ft)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);
        p2p.sendFile(*this,ft);
    }

    void SwitchboardServerConnection::addFileTransferConnectionP2P(FileTransferConnectionP2P *c)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);        
        _fileTransferConnectionsP2P.push_back(c);
    }

    void SwitchboardServerConnection::removeFileTransferConnectionP2P(FileTransferConnectionP2P *c)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);        
        _fileTransferConnectionsP2P.remove(c);
    }

    template <class _Tp>
    class hasCookieOf
    {
        const std::string & cookie;
public:
        hasCookieOf(const std::string & __i) : cookie(__i) {};
        bool operator()(const _Tp &__x) { return __x->cookie == cookie; }
    };
    
    void SwitchboardServerConnection::addCallback(SwitchboardServerCallback callback,
                                                   int trid, void *data)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTING);        
        this->callbacks[trid] = std::make_pair(callback, data);
    }

    void SwitchboardServerConnection::removeCallback(int trid)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTING);        
        this->callbacks.erase(trid);
    }

    void SwitchboardServerConnection::addP2PCallback(SwitchboardServerCallback2 callback,
                                                   int trid, unsigned int sessionID)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTING);        
        this->callbacks2[trid] = std::make_pair(callback, sessionID);
    }

    void SwitchboardServerConnection::removeP2PCallback(int trid)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTING);        
        this->callbacks2.erase(trid);
    }

    void SwitchboardServerConnection::registerMessageHandlers()
    {
        if (messageHandlers.size() == 0)
        {
            messageHandlers["text/plain"]                            = &SwitchboardServerConnection::message_plain;
            messageHandlers["text/x-msmsgsinvite"]                   = &SwitchboardServerConnection::message_invitation;
            messageHandlers["text/x-msmsgscontrol"]                  = &SwitchboardServerConnection::message_typing_user;
            messageHandlers["application/x-msnmsgrp2p"]              = &SwitchboardServerConnection::message_p2p;
            messageHandlers["text/x-msnmsgr-datacast"]               = &SwitchboardServerConnection::message_datacast;
            messageHandlers["text/x-mms-emoticon"]                   = &SwitchboardServerConnection::message_emoticon;
            messageHandlers["text/x-mms-animemoticon"]               = &SwitchboardServerConnection::message_emoticon;
            messageHandlers["image/gif"]                             = &SwitchboardServerConnection::message_ink;
            messageHandlers["application/x-ms-ink"]                  = &SwitchboardServerConnection::message_ink;
        }
    }

    void SwitchboardServerConnection::registerCommandHandlers()
    {
        if (commandHandlers.size() == 0)
        {
            commandHandlers["BYE"] = &SwitchboardServerConnection::handle_BYE;
            commandHandlers["JOI"] = &SwitchboardServerConnection::handle_JOI;
            commandHandlers["NAK"] = &SwitchboardServerConnection::handle_NAK;
            commandHandlers["MSG"] = &SwitchboardServerConnection::handle_MSG;
        }
    }

    void SwitchboardServerConnection::message_plain(std::vector<std::string> & args, std::string mime, std::string body)
    {
        Message msg = Message(body, mime);
        
        this->myNotificationServer()->externalCallbacks.gotInstantMessage(this,args[1], decodeURL(args[2]), &msg);
    }

    void SwitchboardServerConnection::message_emoticon(std::vector<std::string> & args, std::string mime, std::string body)
    {
        std::vector<std::string> emoticons = splitString(body,"\t");
        for(unsigned int i=0; i < emoticons.size(); )
        {
            // avoid errors with clients which do not respect the protocol
            if(i+2 > emoticons.size()) return;
                this->myNotificationServer()->externalCallbacks.gotEmoticonNotification(this,args[1], emoticons[i] , emoticons[i+1]);
            i+=2; // move to the next one
        }
    }

    void SwitchboardServerConnection::message_invitation(std::vector<std::string> & args, std::string mime, std::string body)
    {
    }

    void SwitchboardServerConnection::message_typing_user(std::vector<std::string> & args, std::string mime, std::string body)
    {
        this->myNotificationServer()->externalCallbacks.buddyTyping(this, args[1], decodeURL(args[2]));        
    }   

    void SwitchboardServerConnection::message_ink(std::vector<std::string> & args, std::string mime, std::string body)
    {
        std::string image = body.substr(body.find("base64:")+7);
        this->myNotificationServer()->externalCallbacks.gotInk(this, args[1], image);
    }   

    void SwitchboardServerConnection::message_p2p(std::vector<std::string> & args, std::string mime, std::string body)
    {
        p2p.handleP2Pmessage(*this,args,mime,body);
    }   
    void SwitchboardServerConnection::message_datacast(std::vector<std::string> & args, std::string mime, std::string body)
    {
        Message::Headers headers = Message::Headers(body);
        int id = decimalFromString(headers["ID"]);
        switch(id)
        {
            case 1:
                this->myNotificationServer()->externalCallbacks.gotNudge(this, args[1]);
                break;
            case 2:
                this->myNotificationServer()->externalCallbacks.gotWinkNotification(this, args[1], headers["Data"]);
                break;
            case 3:
                this->myNotificationServer()->externalCallbacks.gotVoiceClipNotification(this, args[1], headers["Data"]);
                break;
            case 4:
                this->myNotificationServer()->externalCallbacks.gotActionMessage(this, args[1], headers["Data"]);
                break;
        }
    }

    void SwitchboardServerConnection::dispatchCommand(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        std::map<std::string, void (SwitchboardServerConnection::*)(std::vector<std::string> &)>::iterator i = commandHandlers.find(args[0]);
        if (i != commandHandlers.end())
            (this->*commandHandlers[args[0]])(args);
    }
    
    int SwitchboardServerConnection::sendMessage(const Message *msg)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);        
        std::string s = msg->asString();
        
        std::ostringstream buf_;
        buf_ << "MSG " << this->trID << " A " << (int) s.size() << "\r\n" << s;
        this->write(buf_);
        this->addCallback(&SwitchboardServerConnection::callback_messageACK, this->trID, NULL);
        return this->trID++;
    }

    int SwitchboardServerConnection::sendMessage(const std::string & body)
    { 
        Message msg(body, "MIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n");
        return this->sendMessage(&msg);
    }    

    void SwitchboardServerConnection::sendKeepAlive()
    {
        this->assertConnectionStateIsAtLeast(SB_READY);
        std::string s("MIME-Version: 1.0\r\nContent-Type: text/x-keepalive\r\n\r\n");
        std::ostringstream buf_;
        buf_ << "MSG " << this->trID++ << " U " << (int) s.size() << "\r\n" << s;
        this->write(buf_);
    }    

    void SwitchboardServerConnection::handle_BYE(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);        
        std::list<Passport> & list = this->users;
        std::list<Passport>::iterator i;
        
        this->myNotificationServer()->externalCallbacks.buddyLeftConversation(this, args[1]);
        
        for (i = list.begin(); i != list.end(); i++)
        {
            if (*i == args[1])
            {
                list.remove(*i);
                break;
            }
        }    
        
        if (this->users.empty() || (args.size() > 3 && args[3] == "1"))
        {
            this->disconnect();
        }
    }
    
    void SwitchboardServerConnection::handle_JOI(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);        
        if (args[1] == this->auth.username)
            return;
        
        if (this->auth.sessionID.empty() && this->connectionState() == SB_WAITING_FOR_USERS)
            this->setConnectionState(SB_READY);
        
        this->users.push_back(args[1]);
        this->myNotificationServer()->externalCallbacks.buddyJoinedConversation(this, args[1], decodeURL(args[2]), 0);
    }
    
    void SwitchboardServerConnection::handle_NAK(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);        
        this->myNotificationServer()->externalCallbacks.failedSendingMessage(this);
    }
    
    void SwitchboardServerConnection::handle_MSG(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        int msglen;
        std::string msg;
        std::string mime;
        std::string body;
        size_t tmp;
        
        msglen = decimalFromString(args[3]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);
        
        body = msg.substr(msg.find("\r\n\r\n") + 4);
        mime = msg.substr(0, msg.size() - body.size());

        std::string contentType;
        Message::Headers headers = Message::Headers(mime);

        std::string chunks = headers["Chunks"];
        if(!chunks.empty())
        {
            // First packet of MultiPacket Data;
            MultiPacketSession session;
            std::string messageID = headers["Message-ID"];
            session.chunks = decimalFromString(chunks);
            session.mime = mime;
            session.receivedChunks = 1;
            session.body += body;
            if(session.chunks != 1)
            {
                MultiPacketSessions[messageID] = session;
                return;
            }

        } 
        else
        {
            std::string chunk = headers["Chunk"];
            if(!chunk.empty())
            {
                // part of multipacket data
                std::string messageID = headers["Message-ID"];
                MultiPacketSession session = MultiPacketSessions[messageID];
                session.body += body;
                session.receivedChunks++;
                if(session.chunks != session.receivedChunks)
                {
                    MultiPacketSessions[messageID] = session;
                    return;
                }
                else
                {
                    MultiPacketSessions.erase(messageID);
                    body = session.body;
                    mime = session.mime;
                    headers = Message::Headers(mime);
                }
            }
        }
        contentType = headers["Content-Type"];

        if ((tmp = contentType.find("; charset")) != std::string::npos)
            contentType = contentType.substr(0, tmp);
        
        std::map<std::string, void (SwitchboardServerConnection::*)(std::vector<std::string> &, std::string, std::string)>::iterator i = messageHandlers.find(contentType);
        if (i != messageHandlers.end())
            (this->*(messageHandlers[contentType]))(args, mime, body);
    }

    void SwitchboardServerConnection::sendAction(std::string action)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);        
        std::ostringstream buf_, msg_;
        msg_ << "MIME-Version: 1.0\r\n";
        msg_ << "Content-Type: text/x-msnmsgr-datacast\r\n\r\n";
        msg_ << "ID: 4\r\n";
        msg_ << "Data: " << action << "\r\n" ;
        size_t msg_length = msg_.str().size();
        
        buf_ << "MSG " << this->trID++ << " U " << (int) msg_length << "\r\n" << msg_.str();
        
        write(buf_);        
    }

    void SwitchboardServerConnection::sendVoiceClip(std::string msnobject)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);
        std::ostringstream buf_, msg_;
        msg_ << "MIME-Version: 1.0\r\n";
        msg_ << "Content-Type: text/x-msnmsgr-datacast\r\n\r\n";
        msg_ << "ID: 3\r\n";
        msg_ << "Data: " << msnobject << "\r\n\r\n";
        size_t msg_length = msg_.str().size();

        buf_ << "MSG " << this->trID++ << " N " << (int) msg_length << "\r\n" << msg_.str();

        write(buf_);
    }
 
    void SwitchboardServerConnection::sendWink(std::string msnobject)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);
        std::ostringstream buf_, msg_;
        msg_ << "MIME-Version: 1.0\r\n";
        msg_ << "Content-Type: text/x-msnmsgr-datacast\r\n\r\n";
        msg_ << "ID: 2\r\n";
        msg_ << "Data: " << msnobject << "\r\n\r\n";
        size_t msg_length = msg_.str().size();

        buf_ << "MSG " << this->trID++ << " N " << (int) msg_length << "\r\n" << msg_.str();

        write(buf_);
    }

    void SwitchboardServerConnection::sendNudge()
    {
        this->assertConnectionStateIsAtLeast(SB_READY);        
        std::ostringstream buf_, msg_;
        msg_ << "MIME-Version: 1.0\r\n";
        msg_ << "Content-Type: text/x-msnmsgr-datacast\r\n\r\n";
        msg_ << "ID: 1\r\n";
        size_t msg_length = msg_.str().size();
        
        buf_ << "MSG " << this->trID++ << " U " << (int) msg_length << "\r\n" << msg_.str();
        
        write(buf_);        
    }

    void SwitchboardServerConnection::sendInk(std::string image)
    {
        this->assertConnectionStateIsAtLeast(SB_READY);

        if(users.size() == 1)
        {
            p2p.sendInk(*this, image);
//              return;
        }

        std::string body("base64:"+image);
        bool one_packet = false;

        if(body.size() <= 1202) // if we need more than 1 packet, then use multipacket
            one_packet = true ;

        if(one_packet)
        {
            std::ostringstream buf_, msg_;
            msg_ << "MIME-Version: 1.0\r\n";
            msg_ << "Content-Type: image/gif\r\n\r\n";
            msg_ << body;

            size_t msg_length = msg_.str().size();
            buf_ << "MSG " << this->trID++ << " N " << (int) msg_length << "\r\n" << msg_.str();
            write(buf_);
            return;
        }
        else
        {
            std::istringstream body_stream(body);
            std::string messageid = new_branch();
            std::vector<std::string> chunks;
            // spliting the message
            while(!body_stream.eof())
            {
                char *part = new char[1203];
                memset(part,0,1203);
                body_stream.read((char*)part, 1202);
                std::string part1(part);
                chunks.push_back(part1);
                delete [] part;
            }

            // sending the first one
            std::ostringstream buf_, msg_;
            msg_ << "MIME-Version: 1.0\r\n";
            msg_ << "Content-Type: image/gif\r\n";
            msg_ << "Message-ID: " << messageid << "\r\n";
            msg_ << "Chunks: " << chunks.size() << "\r\n\r\n";
            msg_ << chunks.front();

            size_t msg_length = msg_.str().size();
            buf_ << "MSG " << this->trID++ << " N " << (int) msg_length << "\r\n" << msg_.str();
            write(buf_);

            std::vector<std::string>::iterator i = chunks.begin();

            for(int num=0; i!=chunks.end(); i++, num++)
            {
                if(i == chunks.begin())
                    continue;

                std::ostringstream buf2_, msg2_;
                msg2_ << "Message-ID: " << messageid << "\r\n";
                msg2_ << "Chunk: " << num << "\r\n\r\n";
                msg2_ << (*i);

                size_t msg_length2 = msg2_.str().size();
                buf2_ << "MSG " << this->trID++ << " N " << (int) msg_length2 << "\r\n" << msg2_.str();
                write(buf2_);
            }
        }
    }
 
    void SwitchboardServerConnection::sendTypingNotification()
    {
        this->assertConnectionStateIsAtLeast(SB_READY);        
        std::ostringstream buf_, msg_;
        msg_ << "MIME-Version: 1.0\r\n";
        msg_ << "Content-Type: text/x-msmsgscontrol\r\n";
        msg_ << "TypingUser: " << this->auth.username << "\r\n";
        msg_ << "\r\n";
        msg_ << "\r\n";
        size_t msg_length = msg_.str().size();
        
        buf_ << "MSG " << this->trID++ << " U " << (int) msg_length << "\r\n" << msg_.str();
        
        write(buf_);        
    }
    
    void SwitchboardServerConnection::inviteUser(Passport userName)
    {
        this->assertConnectionStateIsAtLeast(SB_WAITING_FOR_USERS);        
        std::ostringstream buf_;
        buf_ << "CAL " << this->trID++ << " " << userName << "\r\n";
        write(buf_);        
    }
    
    void SwitchboardServerConnection::connect(const std::string & hostname, unsigned int port)
    {
        this->assertConnectionStateIs(SB_DISCONNECTED);

        if ((this->sock = this->myNotificationServer()->externalCallbacks.connectToServer(hostname, port, &this->connected)) == NULL)
        {
            this->myNotificationServer()->externalCallbacks.showError(this, "Could not connect to switchboard server");
            return;
        }
        
        this->myNotificationServer()->externalCallbacks.registerSocket(this->sock, 0, 1, false);
        this->setConnectionState(SB_CONNECTING);
        
        if (this->connected)
            this->socketConnectionCompleted();
        
        std::ostringstream buf_;
        if (this->auth.sessionID.empty())
        {
            buf_ << "USR " << this->trID << " " << this->auth.username << " " << this->auth.cookie << "\r\n";
            if (this->write(buf_) != buf_.str().size())
                return;
            this->addCallback(&SwitchboardServerConnection::callback_InviteUsers, this->trID, NULL);
        } 
        else
        {
            buf_ << "ANS " << this->trID << " " << this->auth.username << " " << this->auth.cookie << " " << this->auth.sessionID << "\r\n";
            if (this->write(buf_) != buf_.str().size())
                return;
            this->myNotificationServer()->externalCallbacks.gotNewConnection(this);
            this->addCallback(&SwitchboardServerConnection::callback_AnsweredCall, this->trID, NULL);
        }
        
        this->trID++;    
    }
    
    void SwitchboardServerConnection::disconnect()
    {
        if (this->connectionState() == SB_DISCONNECTED)
            return;

        notificationServer.removeSwitchboardConnection(this);
        this->myNotificationServer()->externalCallbacks.closingConnection(this);
        
        std::list<FileTransferConnectionP2P *> list2 = _fileTransferConnectionsP2P;
        std::list<FileTransferConnectionP2P *>::iterator j = list2.begin();
        for (; j != list2.end(); j++)
        {
            removeFileTransferConnectionP2P(*j);
        }

        this->callbacks.clear();
        Connection::disconnect();
        this->setConnectionState(SB_DISCONNECTED);
    }    
    
    void SwitchboardServerConnection::socketConnectionCompleted()
    {
        this->assertConnectionStateIs(SB_CONNECTING);
        Connection::socketConnectionCompleted();
        this->myNotificationServer()->externalCallbacks.unregisterSocket(this->sock);
        this->myNotificationServer()->externalCallbacks.registerSocket(this->sock, 1, 0, false);
        this->setConnectionState(SB_WAITING_FOR_USERS);
    }
    
    void SwitchboardServerConnection::handleIncomingData()
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);        
        while (this->isWholeLineAvailable())
        {                
            std::vector<std::string> args = this->getLine();
            if (args[0] == "MSG" || args[0] == "NOT")
            {
                int dataLength = decimalFromString(args[3]);
                if (this->readBuffer.find("\r\n") + 2 + dataLength > this->readBuffer.size())
                    return;
            }
            this->readBuffer = this->readBuffer.substr(this->readBuffer.find("\r\n") + 2);
            
            int trid = 0;
            
            if (args.size() > 1)
            {
                try
                {
                    trid = decimalFromString(args[1]);
                }
                catch (...)
                {
                }
            }
            
            if (!this->callbacks.empty() && trid > 0)
            {
                if (this->callbacks.find(trid) != this->callbacks.end())
                {
                    (this->*(this->callbacks[trid].first))(args, trid, this->callbacks[trid].second);
                    continue;
                }
            }
            if (!this->callbacks2.empty() && trid > 0)
            {
                if (this->callbacks2.find(trid) != this->callbacks2.end())
                {
                    (this->*(this->callbacks2[trid].first))(args, trid, this->callbacks2[trid].second);
                    continue;
                }
            }
           
            if (isdigit(args[0][0]))
                this->showError(decimalFromString(args[0]));
            else
                this->dispatchCommand(args);
        }
    }
    
    void SwitchboardServerConnection::callback_messageACK(std::vector<std::string> & args, int trid, void *)
    {
        this->removeCallback(trid);
        this->myNotificationServer()->externalCallbacks.gotMessageSentACK(this, trid);
    }

    void SwitchboardServerConnection::callback_InviteUsers(std::vector<std::string> & args, int trid, void *)
    {    
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        this->removeCallback(trid);
        
        if (args.size() < 3)
        {
            this->showError(decimalFromString(args[0]));
            this->disconnect();
            return;
        }

        if(args[2] != "OK")
        {
                this->showError(decimalFromString(args[0]));
                this->disconnect();
                return;
        }
        
        this->myNotificationServer()->externalCallbacks.gotSwitchboard(this, this->auth.tag);
        this->myNotificationServer()->externalCallbacks.gotNewConnection(this);
    }
    
    void SwitchboardServerConnection::callback_continueTransfer(std::vector<std::string> & args, int trid, unsigned int sessionID)
    {
        this->removeP2PCallback(trid);
        p2p.handle_MSGACKReceived(*this,sessionID, args[1]);
    }

    void SwitchboardServerConnection::fileTransferResponse(unsigned int sessionID, std::string filename, bool response)
    {
        p2p.handle_fileTransferResponse(*this,sessionID, filename, response);
    }

    void SwitchboardServerConnection::callback_AnsweredCall(std::vector<std::string> & args, int trid, void * data)
    {
        this->assertConnectionStateIs(SB_WAITING_FOR_USERS);        
        if (args.size() >= 3 && args[0] == "ANS" && args[2] == "OK")
            return;
        
        if (isdigit(args[0][0]))
        {
            this->removeCallback(trid);
            this->showError(decimalFromString(args[0]));
            this->disconnect();
            return;
        }
        
        if (args.size() >= 6 && args[0] == "IRO")
        {
            if (args[4] == this->auth.username)
                return;
            
            this->users.push_back(args[4]);
            this->myNotificationServer()->externalCallbacks.buddyJoinedConversation(this, args[4], decodeURL(args[5]), 1);
            if (args[2] == args[3])
            {
                this->removeCallback(trid);
                this->setConnectionState(SB_READY);
            }
        }
    }
    
    void SwitchboardServerConnection::requestEmoticon(unsigned int id, std::string filename, std::string msnobject, std::string alias)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        p2p.requestEmoticon(*this,id, filename, msnobject, alias);
    }

    void SwitchboardServerConnection::requestWink(unsigned int id, std::string filename, std::string msnobject)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        p2p.requestWink(*this,id, filename, msnobject);
    }

    void SwitchboardServerConnection::requestVoiceClip(unsigned int id, std::string filename, std::string msnobject)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        p2p.requestVoiceClip(*this,id, filename, msnobject);
    }

    void SwitchboardServerConnection::requestDisplayPicture(unsigned int id, std::string filename, std::string msnobject)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        p2p.requestDisplayPicture(*this,id, filename, msnobject);
    }

    void SwitchboardServerConnection::cancelFileTransfer(unsigned int id)
    {
        this->assertConnectionStateIsAtLeast(SB_CONNECTED);
        p2p.cancelTransfer(*this,id);
    }
}
