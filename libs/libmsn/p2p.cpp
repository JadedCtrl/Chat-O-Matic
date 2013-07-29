/*
 * p2p.cpp
 * libmsn
 *
 * Created by Tiago Salem Herrmann on 08/2007.
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

#include <notificationserver.h>
#include <errorcodes.h>
#include <externals.h>
#include <util.h>
#include <p2p.h>
#include <xmlParser.h>

#include <cctype>
#include <iostream>
#include <fstream>
#include <vector>

#include <string.h>

namespace MSN {

    P2P::P2P()
    {
        rand_helper = 1;
    }
    P2P::~P2P()
    {
    }


    void P2P::handleP2Pmessage(MSN::SwitchboardServerConnection &conn, std::vector<std::string> & args, std::string mime, std::string body)
    {
        Message::Headers headers = Message::Headers(mime);
        p2pPacket packet;
        // not for me, ignore it
        if(headers["P2P-Dest"] != conn.myNotificationServer()->myPassport)
            return;

        std::istringstream header(body,std::ios::binary);

        header.read((char*)&packet.p2pHeader.sessionID, sizeof(packet.p2pHeader.sessionID));
        header.read((char*)&packet.p2pHeader.identifier, sizeof(packet.p2pHeader.identifier));
        header.read((char*)&packet.p2pHeader.dataOffset, sizeof(packet.p2pHeader.dataOffset));
        header.read((char*)&packet.p2pHeader.totalDataSize, sizeof(packet.p2pHeader.totalDataSize));
        header.read((char*)&packet.p2pHeader.messageLength, sizeof(packet.p2pHeader.messageLength));
        header.read((char*)&packet.p2pHeader.flag, sizeof(packet.p2pHeader.flag));
        header.read((char*)&packet.p2pHeader.ackID, sizeof(packet.p2pHeader.ackID));
        header.read((char*)&packet.p2pHeader.ackUID, sizeof(packet.p2pHeader.ackUID));
        header.read((char*)&packet.p2pHeader.ackDataSize, sizeof(packet.p2pHeader.ackDataSize));
        char *c = new char[packet.p2pHeader.messageLength];
        header.read(c, packet.p2pHeader.messageLength);
        std::string content(c, packet.p2pHeader.messageLength);
        packet.body = content;
        delete [] c;
        header.read((char*)&packet.p2pFooter.appID, sizeof(packet.p2pFooter.appID));
        
        if(packet.p2pHeader.flag==FLAG_ACK)
        {
            handle_p2pACK(conn, packet);
            return;
        }

        if(packet.p2pHeader.sessionID == 0x40 &&
                little2big_endian(packet.p2pFooter.appID)==3) // INK, oh god!
        {
            p2pSession session;
            session.to = args[1];
            if(!packet.p2pHeader.dataOffset) // first packet
            {
                session.ink = packet.body;
                startedSessions[packet.p2pHeader.sessionID] = session;
                return;
            }
            else
            {
                if (packet.p2pHeader.dataOffset+packet.p2pHeader.messageLength
                        == packet.p2pHeader.totalDataSize)
                {
                    session = startedSessions[packet.p2pHeader.sessionID];
                    session.ink += packet.body;
                    sendACK(conn, packet, session);
                    startedSessions.erase(packet.p2pHeader.sessionID);
                    // TODO - THIS IS UGLY!
                    U8 *d1 = new U8[packet.p2pHeader.totalDataSize];
                    U8 *a = new U8[packet.p2pHeader.totalDataSize];
                    a[0]='\0';
                    a++;
                    const char *f = session.ink.c_str();
                    memcpy(a,f,session.ink.size());
                    a--;
                    _ucs2_utf8(d1, a, session.ink.size());
                    char *c = new char[packet.p2pHeader.totalDataSize+2];
                    char *g = new char[packet.p2pHeader.totalDataSize+2];
                    sprintf(c,"%s",(char*)d1);
                    sprintf(g,"%s",(char*)(d1+strlen((char*)d1)+1));

                    std::string d2(c);
                    std::string d3(g);
                    delete [] a;
                    delete [] c;
                    delete [] d1;
                    delete [] g;

                    conn.message_ink(args, d2, d3);
                    return;
                }
                if (packet.p2pHeader.dataOffset+packet.p2pHeader.messageLength
                        < packet.p2pHeader.totalDataSize)
                {
                    session = startedSessions[packet.p2pHeader.sessionID];
                    session.ink += packet.body;
                    startedSessions[packet.p2pHeader.sessionID] = session;
                    return;
                }
            }
            return;
        }

        // in these conditions, the packet is data, receive it to a file
        if(packet.p2pHeader.sessionID &&
               (packet.p2pHeader.flag == FLAG_FILE_DATA ||
                packet.p2pHeader.flag == FLAG_FILE_DATA2 ||
                packet.p2pHeader.flag == FLAG_DATA_EMOTICONS)) 
        {
            // we need to ensure we have a started session
            if(!startedSessions.count(packet.p2pHeader.sessionID))
                return;

            startedSessions[packet.p2pHeader.sessionID].step = STEP_RECEIVING;
            receiveP2PData(conn,packet);
            return;
        }

        if(packet.p2pHeader.sessionID && packet.p2pFooter.appID)
        {
            // we need to ensure we have a started session
            if(!startedSessions.count(packet.p2pHeader.sessionID))
                return;

            // data preparation, always?
            p2pSession session = startedSessions[packet.p2pHeader.sessionID];
            sendACK(conn, packet, session);
            return;
        }

        if(packet.p2pFooter.appID==APP_NONE) // when 0, assembly all the data before processing
        {
            // reassembly the packet
            if(packet.p2pHeader.messageLength < packet.p2pHeader.totalDataSize) // just part of the packet
            {
                if(pendingP2PMsg.count(packet.p2pHeader.identifier)==0) // it is the first part
                {
                    pendingP2PMsg[packet.p2pHeader.identifier]=packet;
                    return;
                }
                p2pPacket pkt_part = pendingP2PMsg[packet.p2pHeader.identifier];

                // not the first part
                pkt_part.body+=packet.body;

                if(packet.p2pHeader.messageLength+packet.p2pHeader.dataOffset < 
                        packet.p2pHeader.totalDataSize)
                {
                    pendingP2PMsg[packet.p2pHeader.identifier]=pkt_part;
                    // this is not the last part, wait for the last one
                    return;
                }

                // shouldn't be reached
                if(packet.p2pHeader.messageLength+packet.p2pHeader.dataOffset > 
                        packet.p2pHeader.totalDataSize)
                {
                    pendingP2PMsg.erase(packet.p2pHeader.identifier);
                    return;
                }
                if(packet.p2pHeader.messageLength+packet.p2pHeader.dataOffset == 
                        packet.p2pHeader.totalDataSize)
                {
                    packet=pkt_part;
                    pendingP2PMsg.erase(packet.p2pHeader.identifier);
                }
            }
        }

        if(!packet.body.find("INVITE"))
        {
            handle_INVITE(conn,packet);
        }
        else if (!packet.body.find("MSNSLP/1.0 200 OK"))
        {
            handle_200OK(conn,packet);
        }
        else if (!packet.body.find("BYE"))
        {
            handle_BYE(conn,packet);
        }
        else if(!packet.body.find("MSNSLP/1.0 603 Decline") || !packet.body.find("MSNSLP/1.0 603 DECLINE"))
        {
            handle_603Decline(conn,packet);
        }
/*        std::cout << "session id: " << packet.p2pHeader.sessionID << std::endl;
        std::cout << "identifier: " << packet.p2pHeader.identifier << std::endl;
        std::cout << "dataOffset: " << packet.p2pHeader.dataOffset << std::endl;
        std::cout << "totalDataSize: " << packet.p2pHeader.totalDataSize << std::endl;
        std::cout << "messageLength: " << packet.p2pHeader.messageLength << std::endl;
        std::cout << "flag: " << packet.p2pHeader.flag << std::endl;
        std::cout << "ackID: " << packet.p2pHeader.ackID << std::endl;
        std::cout << "ackUID: " << packet.p2pHeader.ackUID << std::endl;
        std::cout << "ackDataSize: " << packet.p2pHeader.ackDataSize << std::endl;
        std::cout << "footer: " << packet.p2pFooter.appID << std::endl << std::endl;
*/
    }

    void P2P::sendACK(MSN::SwitchboardServerConnection &conn, p2pPacket &packet, p2pSession &session)
    {
        p2pPacket ack_pkt;

        std::ostringstream msghdr;
        std::ostringstream footer;
        std::ostringstream header;
        std::ostringstream full_msg;

        session.currentIdentifier++;
        if(session.currentIdentifier == session.baseIdentifier) // skip the original identifier
            session.currentIdentifier++;

        // assembly the ack packet
        ack_pkt.p2pHeader.sessionID = packet.p2pHeader.sessionID;
        ack_pkt.p2pHeader.identifier = session.currentIdentifier;
        ack_pkt.p2pHeader.dataOffset = 0;
        ack_pkt.p2pHeader.totalDataSize = packet.p2pHeader.totalDataSize;
        ack_pkt.p2pHeader.messageLength = 0;
        ack_pkt.p2pHeader.flag = FLAG_ACK;
        ack_pkt.p2pHeader.ackID = packet.p2pHeader.identifier;
        ack_pkt.p2pHeader.ackUID = packet.p2pHeader.ackID;
        ack_pkt.p2pHeader.ackDataSize = packet.p2pHeader.totalDataSize;

        ack_pkt.p2pFooter.appID = APP_NONE;

        msghdr <<"MIME-Version: 1.0\r\n"
            "Content-Type: application/x-msnmsgrp2p\r\n"
            "P2P-Dest: " << conn.users.front() << "\r\n\r\n";

        header.write((char*)&ack_pkt.p2pHeader.sessionID, sizeof(ack_pkt.p2pHeader.sessionID));
        header.write((char*)&ack_pkt.p2pHeader.identifier, sizeof(ack_pkt.p2pHeader.identifier));
        header.write((char*)&ack_pkt.p2pHeader.dataOffset, sizeof(ack_pkt.p2pHeader.dataOffset));
        header.write((char*)&ack_pkt.p2pHeader.totalDataSize, sizeof(ack_pkt.p2pHeader.totalDataSize));
        header.write((char*)&ack_pkt.p2pHeader.messageLength, sizeof(ack_pkt.p2pHeader.messageLength));
        header.write((char*)&ack_pkt.p2pHeader.flag, sizeof(ack_pkt.p2pHeader.flag));
        header.write((char*)&ack_pkt.p2pHeader.ackID, sizeof(ack_pkt.p2pHeader.ackID));
        header.write((char*)&ack_pkt.p2pHeader.ackUID, sizeof(ack_pkt.p2pHeader.ackUID));
        header.write((char*)&ack_pkt.p2pHeader.ackDataSize, sizeof(ack_pkt.p2pHeader.ackDataSize));

        footer.write((char*)&ack_pkt.p2pFooter.appID,sizeof(ack_pkt.p2pFooter.appID));

        full_msg << msghdr.str() << header.str() << footer.str();

        std::ostringstream buf_;
        buf_ << "MSG " << conn.trID++ << " D " << full_msg.str().size() << "\r\n";
        buf_ << full_msg.str();

        if (conn.write(buf_) != buf_.str().size())
            return;
/*            std::cout << "session id: " << ack_pkt.p2pHeader.sessionID << std::endl;
            std::cout << "identifier: " << ack_pkt.p2pHeader.identifier << std::endl;
            std::cout << "dataOffset: " << ack_pkt.p2pHeader.dataOffset << std::endl;
            std::cout << "totalDataSize: " << ack_pkt.p2pHeader.totalDataSize << std::endl;
            std::cout << "messageLength: " << ack_pkt.p2pHeader.messageLength << std::endl;
            std::cout << "flag: " << ack_pkt.p2pHeader.flag << std::endl;
            std::cout << "ackID: " << ack_pkt.p2pHeader.ackID << std::endl;
            std::cout << "ackUID: " << ack_pkt.p2pHeader.ackUID << std::endl;
            std::cout << "ackDataSize: " << ack_pkt.p2pHeader.ackDataSize << std::endl;
            std::cout << "footer: " << ack_pkt.p2pFooter.appID << std::endl << std::endl;
*/

    }

    void P2P::receiveP2PData(MSN::SwitchboardServerConnection &conn, p2pPacket &packet)
    {
        // check if there is no session
        if(!startedSessions.count(packet.p2pHeader.sessionID))
            return;

        p2pSession session = startedSessions[packet.p2pHeader.sessionID];
        if(!session.in_stream && STEP_RECEIVING_FINISHED)
            return;

        if(!session.in_stream->is_open())
        {
            startedSessions[packet.p2pHeader.sessionID].totalDataSize = packet.p2pHeader.totalDataSize;
            session.in_stream->open(session.filename.c_str(), std::ios::binary);
        }

        if(packet.body.length())
            session.in_stream->write(packet.body.c_str(), packet.body.length());

        // notify upper layer the current progress
        if(session.appID == APP_FILE_TRANSFER)
            conn.myNotificationServer()->externalCallbacks.fileTransferProgress(&conn, session.sessionID,session.in_stream->tellp(), packet.p2pHeader.totalDataSize);

        if((unsigned int)packet.p2pHeader.totalDataSize <= session.in_stream->tellp())
        {
            session.in_stream->close();
            session.step = STEP_RECEIVING_FINISHED;
            delete session.in_stream; // end of line
            session.in_stream=NULL;
            sendACK(conn, packet, session);
            startedSessions[packet.p2pHeader.sessionID]=session;

            if(session.appID == APP_DISPLAY_PICTURE ||
               session.appID == APP_DISPLAY_PICTURE2)
            {
                conn.myNotificationServer()->externalCallbacks.gotContactDisplayPicture(&conn, conn.users.front(), session.filename );
            }
            else
            {
                switch(session.typeTransfer)
                {
                    case APP_VOICE_CLIP:
                        libmsn_Siren7_DecodeVoiceClip(session.filename);
                        conn.myNotificationServer()->externalCallbacks.gotVoiceClipFile(&conn, session.sessionID, session.filename);
                        break;
                    case APP_EMOTICON:
                        conn.myNotificationServer()->externalCallbacks.gotEmoticonFile(&conn, session.sessionID, session.emoticonAlias, session.filename);
                        break;
                    case APP_WINK:
                        conn.myNotificationServer()->externalCallbacks.gotWinkFile(&conn, session.sessionID, session.filename);
                        break;
                }
                if(session.appID == APP_FILE_TRANSFER)
                    conn.myNotificationServer()->externalCallbacks.fileTransferSucceeded(&conn, session.sessionID);
            }

            if(session.appID != APP_FILE_TRANSFER)
            {
                send_BYE(conn, packet, session);
                this->addCallback(&P2P::handle_BYEACK, session.sessionID, packet.p2pHeader.ackID);
            }
        }
    }

    void P2P::sendP2PData(MSN::SwitchboardServerConnection &conn, p2pSession &session, p2pPacket &packet)
    {
        p2pPacket pkt_part = session.tempPacket;
        char part[1202];
        std::ostringstream msghdr;
        std::ostringstream footer;
        std::ostringstream header;
        std::ostringstream full_msg;

        msghdr <<"MIME-Version: 1.0\r\n"
            "Content-Type: application/x-msnmsgrp2p\r\n"
            "P2P-Dest: " << conn.users.front() << "\r\n\r\n";
        if(session.tempPacket.p2pHeader.ackID==0) // it means.. first packet
        {
            session.currentIdentifier++;
            if(session.currentIdentifier == session.baseIdentifier) // skip the original identifier
                session.currentIdentifier++;

            session.tempPacket.p2pHeader.sessionID = session.sessionID;
            session.tempPacket.p2pHeader.identifier = session.currentIdentifier;

            if(session.appID == APP_FILE_TRANSFER)
                session.tempPacket.p2pHeader.flag = FLAG_FILE_DATA;
            else
                session.tempPacket.p2pHeader.flag = FLAG_DATA_PICTURE;

            session.tempPacket.p2pHeader.dataOffset = 0;
            session.tempPacket.p2pHeader.totalDataSize = FileSize(session.filename.c_str());
            session.tempPacket.p2pHeader.messageLength = 0;
            session.tempPacket.p2pHeader.ackUID = 0;
            session.tempPacket.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
            session.tempPacket.p2pHeader.ackDataSize=0;
            // swap to big endian
            session.tempPacket.p2pFooter.appID = little2big_endian(session.appID);
            // scheduling the action to do when it is finished
            this->addCallback(&P2P::handle_DataACK, session.sessionID, session.tempPacket.p2pHeader.ackID);
        }

        pkt_part = session.tempPacket;
        if(!session.out_stream)
            return;

        if(!session.out_stream->is_open())
            session.out_stream->open(session.filename.c_str(), std::ios::binary);

        pkt_part.p2pHeader.dataOffset = session.out_stream->tellg();
        session.out_stream->read(part, 1100);

        if(!session.out_stream->gcount())// nothing to read, go away
        {
            session.out_stream->close();
            delete session.out_stream;
            session.out_stream = NULL;
            startedSessions[session.sessionID]=session;
            if(session.appID == APP_FILE_TRANSFER)
                conn.myNotificationServer()->externalCallbacks.fileTransferSucceeded(&conn, session.sessionID);
            return;
        }

        pkt_part.p2pHeader.messageLength = session.out_stream->gcount();

        if(session.appID == APP_FILE_TRANSFER)
            conn.myNotificationServer()->externalCallbacks.fileTransferProgress(&conn, session.sessionID, pkt_part.p2pHeader.dataOffset, pkt_part.p2pHeader.totalDataSize);

        std::string a(part, pkt_part.p2pHeader.messageLength);
        std::istringstream temp_msg(a);

        header.write((char*)&pkt_part.p2pHeader.sessionID, sizeof(pkt_part.p2pHeader.sessionID));
        header.write((char*)&pkt_part.p2pHeader.identifier, sizeof(pkt_part.p2pHeader.identifier));
        header.write((char*)&pkt_part.p2pHeader.dataOffset, sizeof(pkt_part.p2pHeader.dataOffset));
        header.write((char*)&pkt_part.p2pHeader.totalDataSize, sizeof(pkt_part.p2pHeader.totalDataSize));
        header.write((char*)&pkt_part.p2pHeader.messageLength, sizeof(pkt_part.p2pHeader.messageLength));
        header.write((char*)&pkt_part.p2pHeader.flag, sizeof(pkt_part.p2pHeader.flag));
        header.write((char*)&pkt_part.p2pHeader.ackID, sizeof(pkt_part.p2pHeader.ackID));
        header.write((char*)&pkt_part.p2pHeader.ackUID, sizeof(pkt_part.p2pHeader.ackUID));
        header.write((char*)&pkt_part.p2pHeader.ackDataSize, sizeof(pkt_part.p2pHeader.ackDataSize));

        footer.write((char*)&pkt_part.p2pFooter.appID,sizeof(pkt_part.p2pFooter.appID));

        full_msg << msghdr.str() << header.str() << temp_msg.str() << footer.str();

        std::ostringstream buf_;
        buf_ << "MSG " << conn.trID << " D " << full_msg.str().size() << "\r\n";
        buf_ << full_msg.str();

        if (conn.write(buf_) != buf_.str().size())
            return;

        session.tempPacket = pkt_part;

        startedSessions[session.sessionID]=session;
        // call callback_continueTransfer when ack for this packet is received
        conn.addP2PCallback(&SwitchboardServerConnection::callback_continueTransfer, conn.trID++, session.sessionID);
    }

    void P2P::sendP2PPacket(MSN::SwitchboardServerConnection &conn, p2pPacket &packet, p2pSession &session)
    {
        std::ostringstream msghdr;
        std::istringstream msg(packet.body);
        std::ostringstream footer;

        if(session.to.empty())
            session.to = conn.users.front();

        msghdr <<"MIME-Version: 1.0\r\n"
            "Content-Type: application/x-msnmsgrp2p\r\n"
            "P2P-Dest: " << conn.users.front() << "\r\n\r\n";

        footer.write((char*)&packet.p2pFooter.appID,sizeof(packet.p2pFooter.appID));

        session.currentIdentifier++;
        if(session.currentIdentifier == session.baseIdentifier) // skip the original identifier
            session.currentIdentifier++;

        packet.p2pHeader.identifier = session.currentIdentifier;

        // split big messages in many packets
        char temp_buf[1201];
        while(!msg.eof())
        {
            std::ostringstream header;
            std::ostringstream full_msg;
            packet.p2pHeader.dataOffset = msg.tellg();
            msg.read(temp_buf,1200);

            if(msg.gcount()==0)// nothing to read, go away
                break;

            packet.p2pHeader.totalDataSize = msg.str().size();
            packet.p2pHeader.messageLength = msg.gcount();

            std::string a(temp_buf, msg.gcount());
            std::istringstream temp_msg(a);

            header.write((char*)&packet.p2pHeader.sessionID, sizeof(packet.p2pHeader.sessionID));
            header.write((char*)&packet.p2pHeader.identifier, sizeof(packet.p2pHeader.identifier));
            header.write((char*)&packet.p2pHeader.dataOffset, sizeof(packet.p2pHeader.dataOffset));
            header.write((char*)&packet.p2pHeader.totalDataSize, sizeof(packet.p2pHeader.totalDataSize));
            header.write((char*)&packet.p2pHeader.messageLength, sizeof(packet.p2pHeader.messageLength));
            header.write((char*)&packet.p2pHeader.flag, sizeof(packet.p2pHeader.flag));
            header.write((char*)&packet.p2pHeader.ackID, sizeof(packet.p2pHeader.ackID));
            header.write((char*)&packet.p2pHeader.ackUID, sizeof(packet.p2pHeader.ackUID));
            header.write((char*)&packet.p2pHeader.ackDataSize, sizeof(packet.p2pHeader.ackDataSize));

            full_msg << msghdr.str() << header.str() << temp_msg.str() << footer.str();

            std::ostringstream buf_;
            buf_ << "MSG " << conn.trID++ << " D " << full_msg.str().size() << "\r\n";
            buf_ << full_msg.str();

            if (conn.write(buf_) != buf_.str().size())
                return;
        }
    }

    void P2P::handle_session_changes(MSN::SwitchboardServerConnection &conn, p2pPacket &packet, p2pSession &session)
    {
        std::string body;
        std::vector<std::string> msg = splitString(packet.body, "\r\n\r\n");
        msg[1]+="\r\n"; // it is really needed :(
        Message::Headers header_slp = Message::Headers(msg[0]);
        Message::Headers header_app = Message::Headers(msg[1]);
        switch(session.appID)
        {
            case APP_FILE_TRANSFER:
            {
                session.CSeq = decimalFromString(header_slp["CSeq"]);
                session.Bridges = header_app["Bridges"];
                session.NetID = decimalFromString(header_app["NetID"]);
                session.ConnType = header_app["Conn-Type"];
                session.ICF = header_app["ICF"];
                session.UPnPNat = header_app["UPnPNat"];
                session.Listening = header_app["Listening"];

                session.IPv4InternalAddrs = header_app["IPv4Internal-Addrs"];
                session.IPv4InternalPort = header_app["IPv4Internal-Port"];
                session.IPv4ExternalAddrs = header_app["IPv4External-Addrs"];
                session.IPv4ExternalPort = header_app["IPv4External-Port"];

                if(session.step == STEP_RECEIVING)
                {
                    return;
                }

                // direct connection, sending client is waiting connection.
                if(session.Listening == "true")
                {
/*                  // TODO - implement this part
                    bool a;
                    unsigned int port = decimalFromString(session.IPv4ExternalPort);
                    session.fileTransfer = new FileTransferConnectionP2P(conn, session);
                    if(!session.fileTransfer)
                        return;

                    session.fileTransfer->setDirection(FileTransferConnectionP2P::MSNFTP_RECV);
                    session.fileTransfer->setPerspective(FileTransferConnectionP2P::MSNFTP_CLIENT);
                    session.fileTransfer->sock = conn.myNotificationServer()->externalCallbacks.connectToServer(session.IPv4ExternalAddrs, port, &a);
                    conn.addFileTransferConnectionP2P(session.fileTransfer);

                    if (session.fileTransfer->sock < 0)
                    {
                        // if not possible to connect, stop
                        return;
                    }
                    std::ostringstream body2;
                    char b=4;
                    body2.write(&b,1);
                    body2.write("foo\0",4);

                    conn.myNotificationServer()->externalCallbacks.registerSocket(session.fileTransfer->sock,1,1);
                    session.fileTransfer->write(body2.str());*/
                }
                else if (conn.myNotificationServer()->direct_connection)
                {
                    // direct connection, we are waiting connection.
                    // direct_connection means we are directly connected
                    // to the internet
                    body=    "Bridge: TCPv1\r\n"
                        "Listening: true\r\n"
                        "Nonce: {00000000-0000-0000-0000-000000000000}\r\n";
                }
                else
                {
                    // switchboard file transfer, the slowest mode
                    body=    "Bridge: TCPv1\r\n"
                        "Listening: false\r\n"
                        "Nonce: {00000000-0000-0000-0000-000000000000}\r\n";
                }

                send_200OK(conn, session, body);
            }
        }
    }

    void P2P::handle_INVITE(MSN::SwitchboardServerConnection &conn, p2pPacket &packet)
    {
        p2pSession session;
        std::vector<std::string> msg = splitString(packet.body, "\r\n\r\n");
        msg[1]+="\r\n"; // this is really needed :(
        Message::Headers header_slp = Message::Headers(msg[0]);
        Message::Headers header_app = Message::Headers(msg[1]);

        session.to = header_slp["From"];
        session.to = splitString(header_slp["From"], ":")[1];
        session.to = splitString(session.to, ">")[0];
        session.from = header_slp["To"];
        session.from = splitString(header_slp["To"], ":")[1];
        session.from = splitString(session.from, ">")[0];

        session.Via = header_slp["Via"];
        session.CSeq = decimalFromString(header_slp["CSeq"]);
        session.CallID = header_slp["Call-ID"];
        session.ContentType = header_slp["Content-Type"];

        std::map<unsigned int, p2pSession>::iterator i = startedSessions.begin();
        for(; i != startedSessions.end(); i++)
        {
            if((*i).second.CallID == session.CallID)
            {
                // this isn't a new session, since I already have this callid
                p2pSession old_session = (*i).second;
                sendACK(conn, packet, session);
                old_session.Via = session.Via;
                old_session.CSeq = session.CSeq;
                old_session.ContentType = session.ContentType;
                // handle the changes received in this invitation packet
                handle_session_changes(conn, packet, old_session);
                return;
            }
        }
        //session.fromPassport = packet.fromPassport;
        session.sessionID = decimalFromString(header_app["SessionID"]);
        session.appID = decimalFromString(header_app["AppID"]);
        session.Context = header_app["Context"];

        // new connection goes below
        session.out_stream = new std::ifstream;
        session.tempPacket.p2pHeader.ackID=0;
        session.in_stream = NULL;
        session.currentIdentifier = rand()%0x8FFFFFF0 + rand_helper++;
        session.baseIdentifier = session.currentIdentifier; // we need to keep this one
        sendACK(conn, packet, session);
        session.baseIdentifier++;
        session.step = STEP_ACK_INVITATION_SENT;

        switch(session.appID)
        {
            case APP_WEBCAM:
            {
                if(header_app["EUF-GUID"] == "{4BD96FC0-AB17-4425-A14A-439185962DC8}")
                {
                    //conn.myNotificationServer()->externalCallbacks.askWebCam(&conn, session.sessionID);
                    std::string body("SessionID: "+ toStr(session.sessionID) +"\r\n");
                    send_200OK(conn, session, body);
                }
                if(header_app["EUF-GUID"] == "{1C9AA97E-9C05-4583-A3BD-908A196F1E92}")
                {
                    //conn.myNotificationServer()->externalCallbacks.askWebCam(&conn, session.sessionID);
                }
                break;
            }
            case APP_EMOTICON:
            case APP_DISPLAY_PICTURE:
            case APP_DISPLAY_PICTURE2:
            case APP_VOICE_CLIP:
            {
                // I dont know why, just following the rules.
                // I think this is the better place to do this
                session.currentIdentifier-=4;

                std::string body("SessionID: "+ toStr(session.sessionID) +"\r\n");
                send_200OK(conn,session,body); // always accept display picture
                break;
            }
            case APP_FILE_TRANSFER:
            {
                session.currentIdentifier-=1;
                session.sending=false;
                startedSessions[session.sessionID]=session;
                std::istringstream context(b64_decode(session.Context.c_str()), std::ios::binary);

                std::string preview;
                unsigned int header_size;
                unsigned int type;
                bool has_preview =true;

                context.read((char*)&header_size, sizeof(unsigned int));
    
                context.seekg(0, std::ios::beg);
                context.seekg(16);
                context.read((char*)&type, sizeof(unsigned int));
            
                context.seekg(0, std::ios::end);
                // type == 1 means no preview
                if(header_size != context.tellg() && type != 1)
                    has_preview=true;

                context.seekg(0, std::ios::beg);
                context.seekg(19);

                U8 *filenameutf16 = new U8[520];
                U8 *filenameutf8 = new U8[520];
                context.read((char*)filenameutf16, 520);
                _ucs2_utf8(filenameutf8, filenameutf16, 520);
                std::string filename((char*)filenameutf8);
                delete [] filenameutf16;
                delete [] filenameutf8;

                unsigned long long filesize;
                context.seekg(8);
                context.read((char*)&filesize, sizeof(unsigned long long));

                if(has_preview)
                {
                    context.seekg(header_size);
                    int size2 = b64_decode(session.Context.c_str()).size() - header_size;
                    char *a = new char[size2];
                    context.read((char*)a, size2);
                    preview = b64_encode(a,size2);
                    delete [] a;
                }
                fileTransferInvite ft;
                ft.type = type;
                ft.sessionId = session.sessionID;
                ft.userPassport = conn.    users.front();
                ft.filename = filename;
                ft.filesize = filesize;
                ft.preview = preview;

                conn.myNotificationServer()->externalCallbacks.askFileTransfer(&conn, ft);
                return;
            }
        }
    }

    void P2P::handle_200OK(MSN::SwitchboardServerConnection &conn, p2pPacket &packet)
    {
        p2pSession session;
        std::vector<std::string> msg = splitString(packet.body, "\r\n\r\n");
        msg[1]+="\r\n"; // this is really needed :(
        Message::Headers header_slp = Message::Headers(msg[0]);
        Message::Headers header_app = Message::Headers(msg[1]);

        unsigned int tmp_session = decimalFromString(header_app["SessionID"]);

        if(!tmp_session)
            return;

        // we need to ensure we have a started session
        if(!startedSessions.count(tmp_session))
            return;

        session = startedSessions[tmp_session];

        sendACK(conn, packet, session);
        if(session.appID == APP_FILE_TRANSFER)
        {
            sendP2PData(conn, session, packet);
            conn.myNotificationServer()->externalCallbacks.fileTransferInviteResponse(&conn, tmp_session, true);
        }
    }

    void P2P::handle_603Decline(MSN::SwitchboardServerConnection &conn, p2pPacket &packet)
    {
        p2pSession session;
        std::vector<std::string> msg = splitString(packet.body, "\r\n\r\n");
        msg[1]+="\r\n"; // this is really needed :(
        Message::Headers header_slp = Message::Headers(msg[0]);
        Message::Headers header_app = Message::Headers(msg[1]);

        unsigned int tmp_session = decimalFromString(header_app["SessionID"]);

        if(!tmp_session)
            return;

        // we need to ensure we have a started session
        if(!startedSessions.count(tmp_session))
            return;

        session = startedSessions[tmp_session];

        conn.myNotificationServer()->externalCallbacks.fileTransferInviteResponse(&conn, tmp_session, false);
    }

    void P2P::handle_BYE(MSN::SwitchboardServerConnection &conn, p2pPacket &packet)
    {
        p2pSession session;
        std::vector<std::string> msg = splitString(packet.body, "\r\n\r\n");
        if (msg.size() < 2)
        {
            std::cout << "P2P::handle_BYE ERROR size: " << msg.size() << " < 2" << std::endl;
            std::cout << "\'" << packet.body << "\'" << std::endl;
            return;
        }
        msg[1]+="\r\n"; // this is really needed :(
        Message::Headers header_slp = Message::Headers(msg[0]);
        Message::Headers header_app = Message::Headers(msg[1]);

        session.to = header_slp["From"];
        session.to = splitString(header_slp["From"], ":")[1];
        session.to = splitString(session.to, ">")[0];
        session.from = header_slp["To"];
        session.from = splitString(header_slp["To"], ":")[1];
        session.from = splitString(session.from, ">")[0];

        session.CSeq = decimalFromString(header_slp["CSeq"]);
        session.CallID = header_slp["Call-ID"];
        session.Via = header_slp["Via"];

        session.sessionID = decimalFromString(header_app["SessionID"]);
        session.appID = decimalFromString(header_app["AppID"]);
        session.Context = header_app["Context"];
        
        std::map<unsigned int, p2pSession>::iterator i = startedSessions.begin();
        for(; i != startedSessions.end(); i++)
        {
            if((*i).second.CallID == session.CallID)
            {
                sendACK(conn, packet, (*i).second);
                if((*i).second.in_stream && 
                    (unsigned int)(*i).second.totalDataSize > (*i).second.in_stream->tellp())
                {
                    if((*i).second.appID == APP_FILE_TRANSFER)
                        conn.myNotificationServer()->externalCallbacks.fileTransferFailed(&conn, (*i).second.sessionID, MSN::FILE_TRANSFER_ERROR_USER_CANCELED);
                }
                if(!(*i).second.in_stream && (*i).second.appID == APP_FILE_TRANSFER)
                {
                    if((*i).second.appID == APP_FILE_TRANSFER)
                        conn.myNotificationServer()->externalCallbacks.fileTransferFailed(&conn, (*i).second.sessionID, MSN::FILE_TRANSFER_ERROR_USER_CANCELED);
                }
                if((*i).second.in_stream)
                {
                    if((*i).second.in_stream->is_open())
                        (*i).second.in_stream->close();

                    delete (*i).second.in_stream;
                    (*i).second.in_stream = NULL;
                }
                startedSessions.erase((*i).second.sessionID);
                return;
            }
        }
    }

    void P2P::handle_p2pACK(MSN::SwitchboardServerConnection &conn, p2pPacket &packet)
    {

            if (!this->callbacks.empty() && packet.p2pHeader.ackUID > 0)
            {
                if (this->callbacks.find(packet.p2pHeader.ackUID) != this->callbacks.end())
                {
                    (this->*(this->callbacks[packet.p2pHeader.ackUID].first))(conn,this->callbacks[packet.p2pHeader.ackUID].second, packet);
                }
            }
    }

    void P2P::send_200OK(MSN::SwitchboardServerConnection &conn, p2pSession &session, std::string body)
    {
        p2pPacket packet;

        std::ostringstream body2;
        body2.write("\0",1);
        std::string body_final("\r\n"+body+body2.str());

        if(session.ContentType == "application/x-msnmsgr-transreqbody")
            session.ContentType="application/x-msnmsgr-transrespbody";

        std::string content("MSNSLP/1.0 200 OK\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: "+session.Via+"\r\n"
                "CSeq: "+ toStr(++session.CSeq) + "\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: "+session.ContentType +"\r\n"
                "Content-Length: "+ toStr(body_final.length())+ "\r\n"
                +body_final);

        packet.p2pHeader.sessionID = 0;
        packet.p2pHeader.identifier = session.currentIdentifier;
        packet.p2pHeader.flag = FLAG_NOP;
        packet.p2pHeader.dataOffset = 0;
        packet.p2pHeader.totalDataSize = content.length();
        packet.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        packet.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        packet.p2pHeader.ackUID = 0;
        packet.p2pHeader.ackDataSize=0;

        packet.body = content;

        packet.p2pFooter.appID = APP_NONE;

        sendP2PPacket(conn, packet, session);

        session.step = STEP_200OK_SENT;

        startedSessions[session.sessionID]=session;

        this->addCallback(&P2P::handle_200OKACK, session.sessionID, packet.p2pHeader.ackID);
    }

    void P2P::send_BYE(MSN::SwitchboardServerConnection &conn, p2pPacket &packet, p2pSession &session)
    {
        std::ostringstream body;
        body.write("\r\n\0",3);
        std::string content("BYE MSNMSGR:"+session.to+" MSNSLP/1.0\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: "+session.Via+"\r\n"
                "CSeq: 0\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: application/x-msnmsgr-sessionclosebody\r\n"
                "Content-Length: "+ toStr(body.str().size())+ "\r\n"
                +body.str());

        packet.p2pHeader.sessionID = 0;
        packet.p2pHeader.identifier = session.currentIdentifier;
        packet.p2pHeader.flag = FLAG_NOP;
        packet.p2pHeader.dataOffset = 0;
        packet.p2pHeader.totalDataSize = content.length();
        packet.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        packet.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        packet.p2pHeader.ackUID = 0;
        packet.p2pHeader.ackDataSize=0;

        packet.body = content;

        packet.p2pFooter.appID = APP_NONE;

        sendP2PPacket(conn, packet, session);

        session.step = STEP_BYE_SENT;

        startedSessions[session.sessionID]=session;

//        this->addCallback(&P2P::handle_200OKACK, session.sessionID, packet.p2pHeader.ackID);
    }


    void P2P::send_603Decline(MSN::SwitchboardServerConnection &conn, p2pSession &session)
    {
        p2pPacket packet;

        std::ostringstream body2;
        body2.write("\0",1);
        std::string body("\r\nSessionID: "+ toStr(session.sessionID)+"\r\n"+body2.str());
        std::string content("MSNSLP/1.0 603 Decline\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: "+session.Via+"\r\n"
                "CSeq: "+ toStr(++session.CSeq) + "\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
                "Content-Length: "+ toStr(body.length())+ "\r\n"
                +body);

        packet.p2pHeader.sessionID = 0;
        packet.p2pHeader.identifier = session.currentIdentifier;
        packet.p2pHeader.flag = FLAG_NOP;
        packet.p2pHeader.dataOffset = 0;
        packet.p2pHeader.totalDataSize = content.length();
        packet.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        packet.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        packet.p2pHeader.ackUID = 0;
        packet.p2pHeader.ackDataSize=0;

        packet.body = content;

        packet.p2pFooter.appID = APP_NONE;

        sendP2PPacket(conn, packet, session);

        session.step = STEP_603DECLINE_SENT;

        startedSessions[session.sessionID]=session;

        this->addCallback(&P2P::handle_603DeclineACK, session.sessionID, packet.p2pHeader.ackID);
    }


    void P2P::addCallback(P2PCallbacks callback, unsigned int sessionID, unsigned int ackID)
    {
        this->callbacks[ackID] = std::make_pair(callback,sessionID);
    }

    void P2P::removeCallback(unsigned int ackID)
    {
        this->callbacks.erase(ackID);
    }

    void P2P::handle_603DeclineACK(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, p2pPacket &packet)
    {
        this->removeCallback(packet.p2pHeader.ackUID);
        startedSessions.erase(sessionID);
    }

    void P2P::handle_200OKACK(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, p2pPacket &packet)
    {
        p2pPacket pkt_prep;

        this->removeCallback(packet.p2pHeader.ackUID);
        if (startedSessions.find(sessionID) == startedSessions.end())
        {
            return;
        }
        p2pSession session = startedSessions[sessionID];
        session.step = STEP_200OK_ACK_SENT;
        
        switch(session.appID)
        {
            case APP_EMOTICON:
            case APP_WEBCAM:
            case APP_DISPLAY_PICTURE:
            case APP_DISPLAY_PICTURE2:
            case APP_VOICE_CLIP:
            {
                pkt_prep.p2pHeader.sessionID = sessionID;
                pkt_prep.p2pHeader.flag = FLAG_NOP;
                pkt_prep.p2pHeader.identifier = session.currentIdentifier;
                pkt_prep.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
                pkt_prep.p2pHeader.ackUID = 0;
                pkt_prep.p2pHeader.ackDataSize = 0;
                // big endian
                pkt_prep.p2pFooter.appID = little2big_endian(session.appID);

                std::ostringstream content;
                content.write("\0\0\0\0",4);
                pkt_prep.body = content.str();

                sendP2PPacket(conn, pkt_prep, session);
                session.step = STEP_DATA_PREPARATION_SENT;
                session.typeTransfer = (p2pTransferObj)session.appID;

                startedSessions[sessionID]=session;

                this->addCallback(&P2P::handle_DataPreparationACK, session.sessionID, pkt_prep.p2pHeader.ackID);
                break;
            }
            case APP_FILE_TRANSFER:
            {
                // great, on 200OK from file transfer the packet does not 
                // have appID
                // We should wait now an inviting for direct connection
            }
        }
    }

    void P2P::handle_DataPreparationACK(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, p2pPacket &packet)
    {
        this->removeCallback(packet.p2pHeader.ackUID);
        p2pSession session = startedSessions[sessionID];
        session.step = STEP_SENDING;
        std::string filepath;
        filepath += b64_decode(session.Context.c_str()); // prevents empty context
        if(filepath.length())
        {
            if(!conn.myNotificationServer()->msnobj.getMSNObjectRealPath(b64_decode(session.Context.c_str()), session.filename))
            {
                send_603Decline(conn,session);
                return;
            }
        }
        else
        {
            send_603Decline(conn,session);
            return;
        }
        sendP2PData(conn, session, packet);
    }

    void P2P::handle_MSGACKReceived(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string fromPassport)
    {
        p2pPacket packet;
        if(startedSessions.find(sessionID) == startedSessions.end())
            return;

        p2pSession session = startedSessions[sessionID];
        sendP2PData(conn, session, packet);
    }

    void P2P::handle_fileTransferResponse(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string filename, bool response)
    {
        p2pSession session = startedSessions[sessionID];
        session.filename = filename;
        if(response) // user accepted
        {
            session.in_stream = new std::ofstream;
            std::string body("SessionID: "+ toStr(session.sessionID) +"\r\n");
            send_200OK(conn, session, body);
        }
        else // user rejected
        {
            // I dont want to receive your file, blergh
            send_603Decline(conn,session);
        }
    }

    void P2P::handle_DataACK(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, p2pPacket &packet)
    {
        this->removeCallback(packet.p2pHeader.ackUID);

        p2pPacket pkt_bye;

        std::string branch = new_branch();

        p2pSession session = startedSessions[sessionID];
        session.step = STEP_DATA_TRANSFER_ACK;

        std::ostringstream body;
        body.write("\r\n\0",3);
        std::string content("BYE MSNMSGR:"+session.to+" MSNSLP/1.0\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: MSNSLP/1.0/TLP ;branch="+branch+"\r\n"
                "CSeq: 0\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: application/x-msnmsgr-sessionclosebody\r\n"
                "Content-Length: "+ toStr(body.str().length())+ "\r\n"
                +body.str());

        pkt_bye.p2pHeader.sessionID = 0;
        pkt_bye.p2pHeader.identifier = session.currentIdentifier;
        pkt_bye.p2pHeader.flag = FLAG_NOP;
        pkt_bye.p2pHeader.dataOffset = 0;
        pkt_bye.p2pHeader.totalDataSize = content.length();
        pkt_bye.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        pkt_bye.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        pkt_bye.p2pHeader.ackUID = 0;
        pkt_bye.p2pHeader.ackDataSize=0;

        pkt_bye.body = content;

        pkt_bye.p2pFooter.appID = APP_NONE;

        sendP2PPacket(conn, pkt_bye, session);

        session.step = STEP_BYE_SENT;

        startedSessions[session.sessionID]=session;

        this->addCallback(&P2P::handle_BYEACK, session.sessionID, pkt_bye.p2pHeader.ackID);
    }

    void P2P::handle_BYEACK(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, p2pPacket &packet)
    {
        this->removeCallback(packet.p2pHeader.ackUID);
    }

    void P2P::requestDisplayPicture(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string filename, std::string msnobject)
    {
        p2pSession session;
        session.Context = b64_encode(msnobject.c_str(), msnobject.size());
        session.CSeq = 0;
        session.sessionID = sessionID;
        session.filename = filename;
        session.CallID = new_branch();
        session.to = conn.users.front() ;
        session.from = conn.myNotificationServer()->myPassport;
        session.currentIdentifier = rand()%0x8FFFFFF0 + rand_helper++;
        session.baseIdentifier = session.currentIdentifier;
        session.Via = "MSNSLP/1.0/TLP ;branch=";
        session.Via+= new_branch();
        session.appID = APP_DISPLAY_PICTURE2;
        p2pPacket packet;
        std::ostringstream body2;
        body2.write("\0",1);
        std::string body("\r\n"
                "EUF-GUID: {A4268EEC-FEC5-49E5-95C3-F126696BDBF6}\r\n"
                "SessionID: "+ toStr(session.sessionID)+"\r\n"
                "AppID: 1\r\n"
                "Context: " + session.Context + "\r\n"+body2.str());
        std::string content("INVITE MSNMSGR:"+ session.to +" MSNSLP/1.0\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: "+session.Via+"\r\n"
                "CSeq: "+ toStr(session.CSeq++) + "\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
                "Content-Length: "+ toStr(body.length())+ "\r\n"
                +body);

        packet.p2pHeader.sessionID = 0;
        packet.p2pHeader.identifier = session.currentIdentifier;
        packet.p2pHeader.flag = FLAG_NOP;
        packet.p2pHeader.dataOffset = 0;
        packet.p2pHeader.totalDataSize = content.length();
        packet.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        packet.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        packet.p2pHeader.ackUID = 0;
        packet.p2pHeader.ackDataSize=0;

        packet.body = content;

        packet.p2pFooter.appID = APP_NONE;

        session.in_stream = new std::ofstream;

        sendP2PPacket(conn, packet, session);

        session.currentIdentifier = session.currentIdentifier - 3;

        startedSessions[session.sessionID]=session;
    }

    void P2P::requestFile(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string filename, std::string msnobject, p2pTransferObj obj)
    {
        p2pSession session;
        if(startedSessions.find(sessionID) != startedSessions.end())
            session = startedSessions[sessionID];

        session.Context = b64_encode(msnobject.c_str(), msnobject.size());
        session.CSeq = 0;
        session.sessionID = sessionID;
        session.filename = filename;
        session.CallID = new_branch();
        session.to = conn.users.front() ;
        session.from = conn.myNotificationServer()->myPassport;
        session.currentIdentifier = rand()%0x8FFFFFF0 + rand_helper++;
        session.baseIdentifier = session.currentIdentifier;
        session.Via = "MSNSLP/1.0/TLP ;branch=";
        session.Via+= new_branch();
        session.typeTransfer = obj;

        p2pPacket packet;
        std::ostringstream body2;
        body2.write("\0",1);
        std::string body("\r\n"
                "EUF-GUID: {A4268EEC-FEC5-49E5-95C3-F126696BDBF6}\r\n"
                "SessionID: "+ toStr(session.sessionID)+"\r\n"
                "AppID: 1\r\n"
                "Context: " + session.Context + "\r\n"+body2.str());
        std::string content("INVITE MSNMSGR:"+ session.to +" MSNSLP/1.0\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: "+session.Via+"\r\n"
                "CSeq: "+ toStr(session.CSeq++) + "\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
                "Content-Length: "+ toStr(body.length())+ "\r\n"
                +body);

        packet.p2pHeader.sessionID = 0;
        packet.p2pHeader.identifier = session.currentIdentifier;
        packet.p2pHeader.flag = FLAG_NOP;
        packet.p2pHeader.dataOffset = 0;
        packet.p2pHeader.totalDataSize = content.length();
        packet.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        packet.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        packet.p2pHeader.ackUID = 0;
        packet.p2pHeader.ackDataSize=0;

        packet.body = content;

        packet.p2pFooter.appID = APP_NONE;

        session.in_stream = new std::ofstream;

        sendP2PPacket(conn, packet, session);

        session.currentIdentifier = session.currentIdentifier - 3;

        startedSessions[session.sessionID]=session;
    }

    void P2P::requestEmoticon(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string filename, std::string msnobject, std::string alias)
    {
        p2pSession session;
        session.emoticonAlias = alias;
        startedSessions[sessionID]=session;
        requestFile(conn, sessionID, filename, msnobject, APP_EMOTICON);
    }

    void P2P::requestVoiceClip(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string filename, std::string msnobject)
    {
        requestFile(conn, sessionID, filename, msnobject, APP_VOICE_CLIP);
    }

    void P2P::requestWink(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, std::string filename, std::string msnobject)
    {
        requestFile(conn, sessionID, filename, msnobject, APP_WINK);
    }

    void P2P::handle_INVITE_ACK(MSN::SwitchboardServerConnection &conn, unsigned int sessionID, p2pPacket &packet)
    {
    }

    std::string build_file_transfer_context(MSN::fileTransferInvite ft)
    {
        std::ostringstream context;
        unsigned int hlength=0x27E;
        unsigned int msnversion=0x03;
        unsigned long long filesize = FileSize(ft.filename.c_str());
        unsigned int type = ft.type;
        char filename[520];
        char unknown1[30];
        unsigned int unknown2 = (ft.type == FILE_TRANSFER_BACKGROUND_SHARING) ||
               (ft.type == FILE_TRANSFER_BACKGROUND_SHARING_CUSTOM) ? 0xFFFFFE : 0xFFFFFF ;
        char unknown3[64];

        memset(&filename,0,520);
        memset(&unknown1,0,30);
        memset(&unknown3,0,64);

        // TODO - convert filename to ucs2
        U8 *filenameutf8 = new U8[520];
        U8 *filenameutf16 = new U8[521];
        memset(filenameutf8, 0, 520);
        memset(filenameutf16, 0, 521);
        memcpy(filenameutf8, ft.friendlyname.c_str(), ft.friendlyname.size());
        _utf8_ucs2(filenameutf16, filenameutf8);
        filenameutf16++;

        context.write((char*)&hlength, sizeof(unsigned int));
        context.write((char*)&msnversion, sizeof(unsigned int));
        context.write((char*)&filesize, sizeof(unsigned long long));
        context.write((char*)&type, sizeof(unsigned int));
        context.write((char*)filenameutf16, 520);
        context.write((char*)&unknown1, 30);
        context.write((char*)&unknown2, sizeof(unsigned int));
        context.write((char*)&unknown3, 64);

        filenameutf16--;
        delete [] filenameutf16;
        delete [] filenameutf8;

        if(ft.type == FILE_TRANSFER_WITH_PREVIEW && ft.preview.size())
            context.write((char*)b64_decode(ft.preview.c_str()).c_str(), b64_decode(ft.preview.c_str()).size());

        return b64_encode(context.str().c_str(), context.str().size());
    }

    void P2P::sendFile(MSN::SwitchboardServerConnection &conn, MSN::fileTransferInvite ft)
    {
        p2pSession session;
        session.Context = build_file_transfer_context(ft);
        session.CSeq = 0;
        session.sessionID = ft.sessionId;
        session.filename = ft.filename;
        session.CallID = new_branch();
        session.to = conn.users.front();
        session.from = conn.myNotificationServer()->myPassport;
        session.currentIdentifier = rand()%0x8FFFFFF0 + rand_helper++;
        session.baseIdentifier = session.currentIdentifier;
        session.Via = "MSNSLP/1.0/TLP ;branch=";
        session.Via+= new_branch();
        session.tempPacket.p2pHeader.ackID=0;
        session.in_stream = NULL;

        p2pPacket packet;
        std::ostringstream body2;
        body2.write("\0",1);
        std::string other_passport = conn.users.front();
        std::string body("\r\n"
                "EUF-GUID: {5D3E02AB-6190-11D3-BBBB-00C04F795683}\r\n"
                "SessionID: "+ toStr(session.sessionID)+"\r\n"
                "SChannelState: 0\r\n"
                "Capabilities-Flags: 1\r\n"
                "AppID: 2\r\n"
                "Context: " + session.Context + "\r\n"+body2.str());
        std::string content("INVITE MSNMSGR:"+ other_passport +" MSNSLP/1.0\r\n"
                "To: <msnmsgr:"+session.to+">\r\n"
                "From: <msnmsgr:"+session.from+">\r\n"
                "Via: "+session.Via+"\r\n"
                "CSeq: "+ toStr(session.CSeq++) + "\r\n"
                "Call-ID: "+session.CallID+"\r\n"
                "Max-Forwards: 0\r\n"
                "Content-Type: application/x-msnmsgr-sessionreqbody\r\n"
                "Content-Length: "+ toStr(body.length())+ "\r\n"
                +body);

        packet.p2pHeader.sessionID = 0;
        packet.p2pHeader.identifier = session.currentIdentifier;
        packet.p2pHeader.flag = FLAG_NOP;
        packet.p2pHeader.dataOffset = 0;
        packet.p2pHeader.totalDataSize = content.length();
        packet.p2pHeader.messageLength = 0; // filled  inside sendP2PPacket()
        packet.p2pHeader.ackID = rand()%0x8FFFFFF0 + rand_helper++;
        packet.p2pHeader.ackUID = 0;
        packet.p2pHeader.ackDataSize=0;

        packet.body = content;

        packet.p2pFooter.appID = APP_NONE;
        session.appID = APP_FILE_TRANSFER;

        session.out_stream = new std::ifstream;

        sendP2PPacket(conn, packet, session);

        startedSessions[session.sessionID]=session;
    }

    void P2P::sendInk(MSN::SwitchboardServerConnection &conn, std::string image)
    {
        std::list<Passport>::iterator i = conn.users.begin();
        // for each user

    }
    void P2P::cancelTransfer(MSN::SwitchboardServerConnection &conn, unsigned int sessionID)
    {
        p2pSession session;
        p2pPacket packet;
        if(startedSessions.find(sessionID) == startedSessions.end())
            return;

        session = startedSessions[sessionID];
        send_BYE(conn, packet, session);
        startedSessions.erase(sessionID);
    }
}
