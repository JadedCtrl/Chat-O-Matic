#ifndef __msn_p2p_h__
#define __msn_p2p_h__
/*
 * p2p.h
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


#include <connection.h>
#include <authdata.h>
#include <errorcodes.h>
#include <buddy.h>
#include <passport.h>
#include <util.h>
#include <stdexcept>

#include <vector>
#include <fstream>
#include <iostream>

#include "libmsn_export.h"

namespace MSN
{
    class SwitchboardServerConnection;
    class FileTransferConnectionP2P;

    /** Manages all p2p communication. 
     */
    class LIBMSN_EXPORT P2P
    {
        public:
            P2P();
            virtual ~P2P();
            unsigned int rand_helper;
            enum {
                DIRECTION_SENDING = 0,
                DIRECTION_RECEIVING = 1
            };
            enum {
                STEP_INVITATION_SENT,
                STEP_ACK_INVITATION_SENT,
                STEP_200OK_SENT,
                STEP_200OK_ACK_SENT,
                STEP_603DECLINE_SENT,
                STEP_603DECLINE_ACK_SENT,
                STEP_DC_INVITE_SENT, // direct connection
                STEP_DC_INVITE_ACK_SENT, // direct connection
                STEP_DC_200OK_SENT,
                STEP_DC_200OKACK_SENT,
                STEP_DATA_PREPARATION_SENT,
                STEP_DATA_PREPARATION_ACK,
                STEP_SENDING,
                STEP_RECEIVING,
                STEP_RECEIVING_FINISHED,
                STEP_DATA_TRANSFER_ACK,
                STEP_BYE_SENT,
                STEP_BYE_ACK
            };
            typedef enum {
                APP_NONE = 0,
                APP_WEBCAM = 4,
                APP_FILE_TRANSFER = 2,
                APP_DISPLAY_PICTURE = 1,
                APP_EMOTICON = 11,
                APP_DISPLAY_PICTURE2 = 12, // MSNP15 uses 12 instead 1
                APP_VOICE_CLIP = 20, // MSNP15 uses 12 instead 1
                APP_WINK = 98, // non standard
                APP_INK = 99 // non standard
            } p2pTransferObj;
            enum {
                FLAG_NOP = 0x0,
                FLAG_ACK = 0x2,
                FLAG_ERROR = 0x8,
                FLAG_DATA_EMOTICONS = 0x20,
                FLAG_DATA_PICTURE = 0x20,
                FLAG_FILE_DATA = 0x01000030,
                FLAG_FILE_DATA2 = 0x01000020
            };
            
            struct p2pPacket {
                struct {
                    unsigned int sessionID;
                    unsigned int identifier;
                    unsigned long long dataOffset;
                    unsigned long long totalDataSize;
                    unsigned int messageLength;
                    unsigned int flag;
                    unsigned int ackID;
                    unsigned int ackUID;
                    unsigned long long ackDataSize;
                }p2pHeader;
                std::string body;
                struct {
                    unsigned int appID;
                }p2pFooter;

                p2pPacket() {
                    p2pHeader.sessionID = 0;
                    p2pHeader.identifier = 0;
                    p2pHeader.dataOffset = 0;
                    p2pHeader.totalDataSize = 0;
                    p2pHeader.messageLength = 0;
                    p2pHeader.flag = 0;
                    p2pHeader.ackID = 0;
                    p2pHeader.ackUID = 0;
                    p2pHeader.ackDataSize = 0;
                    p2pFooter.appID = 0;
                }
            };

            struct p2pSession {
                bool sending; // sending or receiving, if sending, so true
                unsigned long long totalDataSize;
                unsigned int step; // step at the moment
                unsigned int currentIdentifier;
                unsigned int baseIdentifier; // baseIdentifier
                unsigned int CSeq;
                unsigned int sessionID;
                unsigned int appID;
                MSN::FileTransferConnectionP2P *fileTransfer;
                std::string from;
                std::string to;
                std::string CallID;
                std::string Via;
                std::string ContentType;
                std::string Context; // can be the file preview or msnobject
                std::string filename; // filename to transfer
                std::ifstream *out_stream; // file to send
                std::ofstream *in_stream; // file to receive
                std::string ConnType;
                std::string Bridges;
                std::string NetID;
                std::string UPnPNat;
                std::string Listening;
                std::string ICF;
                std::string IPv4InternalAddrs;
                std::string IPv4InternalPort;
                std::string IPv4ExternalAddrs;
                std::string IPv4ExternalPort;
                p2pTransferObj typeTransfer;
                std::string emoticonAlias;

                p2pPacket tempPacket; // this is used for general purposes
                std::string ink;

                p2pSession() {
                    sending = false;
                    totalDataSize = 0;
                    step = 0;
                    currentIdentifier = 0;
                    baseIdentifier = 0;
                    CSeq = 0;
                    sessionID = 0;
                    appID = 0;
                    fileTransfer = 0;
                    out_stream = 0;
                    in_stream = 0;
                    typeTransfer = APP_NONE;
                }
            };

            typedef void (P2P::*P2PCallbacks)(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            std::map<unsigned int, std::pair<P2PCallbacks, unsigned int> > callbacks;

            virtual void addCallback(P2PCallbacks, unsigned int sessionID, 
                    unsigned int ackID);

            virtual void removeCallback(unsigned int ackID);

            std::map<unsigned int, p2pPacket> pendingP2PMsg;
            std::map<unsigned int, p2pSession> startedSessions;

            void sendFile(MSN::SwitchboardServerConnection &conn, 
                          MSN::fileTransferInvite ft);

            void handleP2Pmessage(MSN::SwitchboardServerConnection &conn, 
                          std::vector<std::string> & args, 
                          std::string mime, std::string body);

            void sendACK(MSN::SwitchboardServerConnection &conn, 
                      p2pPacket &packet,
                      p2pSession &session);

            void sendP2PPacket(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet, 
                    p2pSession &session);

            void sendP2PData(MSN::SwitchboardServerConnection &conn, 
                    p2pSession &session,
                    p2pPacket &packet);

            void receiveP2PData(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void handle_negotiation(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void handle_INVITE(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void handle_603Decline(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void handle_INVITE_ACK(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            void handle_200OK(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void handle_BYE(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void send_200OK(MSN::SwitchboardServerConnection &conn, 
                    p2pSession &session,
                    std::string body);

            void send_BYE(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet,
                    p2pSession &session);

            void send_603Decline(MSN::SwitchboardServerConnection &conn, 
                    p2pSession &session);

            void handle_p2pACK(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet);

            void handle_200OKACK(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            void handle_603DeclineACK(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            void handle_DataPreparationACK(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            void handle_DataACK(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            void handle_BYEACK(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    p2pPacket &packet);

            void handle_MSGACKReceived(MSN::SwitchboardServerConnection &conn,
                    unsigned int sessionID,
                    std::string fromPassport);

            void handle_fileTransferResponse(MSN::SwitchboardServerConnection &conn,
                    unsigned int sessionID,
                    std::string filename,
                    bool response);

            void handle_session_changes(MSN::SwitchboardServerConnection &conn, 
                    p2pPacket &packet, 
                    p2pSession &session);

            void requestFile(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    std::string filename, 
                    std::string msnobject,
                    p2pTransferObj obj);

            void requestDisplayPicture(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    std::string filename, 
                    std::string msnobject);

            void sendInk(MSN::SwitchboardServerConnection &conn, 
                    std::string image);

            void cancelTransfer(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID);

            void requestEmoticon(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    std::string filename, 
                    std::string msnobject,
                    std::string alias);

            void requestVoiceClip(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    std::string filename, 
                    std::string msnobject);

            void requestWink(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    std::string filename, 
                    std::string msnobject);

            void requestInk(MSN::SwitchboardServerConnection &conn, 
                    unsigned int sessionID, 
                    std::string filename, 
                    std::string msnobject);
    };
}

#endif
