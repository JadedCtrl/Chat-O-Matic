/*
 * notificationserver.cpp
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

#include "config.h"
#include "notificationserver.h"
#include "errorcodes.h"
#include "externals.h"
#include "md5.h"
#include "util.h"
#include "soap.h"
#include <algorithm>
#include <cctype>
#include <cassert>

#ifndef WIN32
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#else
#include <io.h>
#endif

#include <stdio.h>
#include <string.h>
#include <map>

#include "xmlParser.h"

namespace MSN
{
    std::map<std::string, void (NotificationServerConnection::*)(std::vector<std::string> &)> NotificationServerConnection::commandHandlers;
    std::map<std::string, void (NotificationServerConnection::*)(std::vector<std::string> &, std::string, std::string)> NotificationServerConnection::messageHandlers;

    NotificationServerConnection::NotificationServerConnection(Passport username_, std::string password_, Callbacks & cb_) 
        : Connection(), auth(username_, password_), myPassport(username_), m_clientId(0), externalCallbacks(cb_), _connectionState(NS_DISCONNECTED), generatingLockkey(false), removingOIM(false), bplSetting('B')
    {
        msnobj.setCreator(username_);
        registerHandlers();
    }
    
    NotificationServerConnection::~NotificationServerConnection()
    {
        if (this->connectionState() != NS_DISCONNECTED)
            this->disconnect();
    }
    
    Connection *NotificationServerConnection::connectionWithSocket(void *sock)
    {
        if (this->sock == sock)
            return this;

        std::vector<SwitchboardServerConnection *> & list = _switchboardConnections;
        std::vector<SwitchboardServerConnection *>::iterator i = list.begin();
      
        for (; i != list.end(); i++)
        {
            Connection *c = (*i)->connectionWithSocket(sock);
            if (c)
                return c;
        }

        std::vector<Soap *> & list2 = _SoapConnections;
        std::vector<Soap *>::iterator d = list2.begin();

        for (; d != list2.end(); d++)
        {
            if((*d)->sock == sock )
                return (*d);
        }


        return NULL;
    }
    
    SwitchboardServerConnection *NotificationServerConnection::switchboardWithOnlyUser(Passport username)
    {
        if (this->connectionState() >= NS_CONNECTED)
        {
            std::vector<SwitchboardServerConnection *> & list = _switchboardConnections;
            std::vector<SwitchboardServerConnection *>::iterator i = list.begin();
            
            for (; i != list.end(); i++)
            {
                if ((*i)->users.size() == 1 &&
                    *((*i)->users.begin()) == username)
                    return *i;
            }
        }
        return NULL;
    }
    
    const std::vector<SwitchboardServerConnection *> & NotificationServerConnection::switchboardConnections()
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        return _switchboardConnections;
    }
    
    void NotificationServerConnection::addSwitchboardConnection(SwitchboardServerConnection *c)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);        
        _switchboardConnections.push_back(c);
    }

    void NotificationServerConnection::addSoapConnection(Soap *s)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        _SoapConnections.push_back(s);
    }

    void NotificationServerConnection::removeSwitchboardConnection(SwitchboardServerConnection *c)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);        
        std::vector<SwitchboardServerConnection *>::iterator it;
        for(it = _switchboardConnections.begin(); it != _switchboardConnections.end(); it++)
        {
            if((*it) == c)
            {
                _switchboardConnections.erase(it);
                return;
            }
        }
    }

    void NotificationServerConnection::removeSoapConnection(Soap *s)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);       
        std::vector<Soap *>::iterator it;
        for(it = _SoapConnections.begin(); it != _SoapConnections.end(); it++)
        {
            if((*it) == s)
            {
                _SoapConnections.erase(it);
                return;
            }
        }
    }
   
    void NotificationServerConnection::addCallback(NotificationServerCallback callback,
                                                   int trid, void *data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTING);        
        this->callbacks[trid] = std::make_pair(callback, data);
    }
    
    void NotificationServerConnection::removeCallback(int trid)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTING);        
        this->callbacks.erase(trid);
    }
    
    void NotificationServerConnection::registerHandlers()
    {
        if (commandHandlers.size() == 0)
        {
            commandHandlers["OUT"] = &NotificationServerConnection::handle_OUT;
            commandHandlers["RML"] = &NotificationServerConnection::handle_RML;
            commandHandlers["BLP"] = &NotificationServerConnection::handle_BLP;
            commandHandlers["CHG"] = &NotificationServerConnection::handle_CHG;
            commandHandlers["CHL"] = &NotificationServerConnection::handle_CHL;
            commandHandlers["ILN"] = &NotificationServerConnection::handle_ILN;
            commandHandlers["NLN"] = &NotificationServerConnection::handle_NLN;
            commandHandlers["FLN"] = &NotificationServerConnection::handle_FLN;
            commandHandlers["MSG"] = &NotificationServerConnection::handle_MSG;
            commandHandlers["PRP"] = &NotificationServerConnection::handle_PRP;
            commandHandlers["UBX"] = &NotificationServerConnection::handle_UBX;
            commandHandlers["GCF"] = &NotificationServerConnection::handle_GCF;
            commandHandlers["ADL"] = &NotificationServerConnection::handle_ADL;
            commandHandlers["UBN"] = &NotificationServerConnection::handle_UBN;
            commandHandlers["FQY"] = &NotificationServerConnection::handle_FQY;
        }

        if (messageHandlers.size() == 0)
        {
                messageHandlers["text/x-msmsgsinitialemailnotification"] = &NotificationServerConnection::message_initial_email_notification;
                messageHandlers["text/x-msmsgsinitialmdatanotification"] = &NotificationServerConnection::message_initialmdatanotification;
                messageHandlers["text/x-msmsgsemailnotification"]        = &NotificationServerConnection::message_email_notification;
                messageHandlers["text/x-msmsgsprofile"]                  = &NotificationServerConnection::message_msmsgsprofile;
                messageHandlers["text/x-msmsgsoimnotification"]          = &NotificationServerConnection::message_oimnotification;
        }
    }

    void NotificationServerConnection::dispatchCommand(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);        
        std::map<std::string, void (NotificationServerConnection::*)(std::vector<std::string> &)>::iterator i = commandHandlers.find(args[0]);
        if (i != commandHandlers.end())
            (this->*commandHandlers[args[0]])(args);
    }

    void NotificationServerConnection::disconnectNS()
    {
        std::ostringstream buf_;
        buf_ << "OUT\r\n";
        if (this->write(buf_) != buf_.str().size())
            return;
        disconnect();
    }

    void NotificationServerConnection::handle_OUT(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);        
        if (args.size() > 1)
        {
            if (args[1] == "OTH")
            {
                this->myNotificationServer()->externalCallbacks.showError(this, "You have logged onto MSN twice at once. Your MSN session will now terminate.");
            }
            else if (args[1] == "SSD")
            {
                this->myNotificationServer()->externalCallbacks.showError(this, "This MSN server is going down for maintenance. Your MSN session will now terminate.");
            } else {
                this->myNotificationServer()->externalCallbacks.showError(this, (std::string("The MSN server has terminated the connection with an unknown reason code. Please report this code: ") + 
                                      args[1]).c_str());
            }
        }
        this->disconnect();
    }
    
    void NotificationServerConnection::handle_RML(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        int msglen;
        std::string msg;

        if(args[2] != "OK" && args[2] != "OK")
            return; // TODO - raise an error

        msglen = decimalFromString(args[2]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);

        XMLNode rml_data = XMLNode::parseString( msg.c_str() );

        int nDomains = rml_data.nChildNode("d");
        for(int i=0; i<nDomains; i++)
        {
            XMLNode domain = rml_data.getChildNode("d", i);
            std::string domain_name = domain.getAttribute("n",0);
            int nContacts =  domain.nChildNode("c");
            for(int j=0; j< nContacts; j++)
            {
                XMLNode contact = domain.getChildNode("c",j);
                std::string contact_name = contact.getAttribute("n",0);
                MSN::ContactList list_number = (MSN::ContactList) decimalFromString(contact.getAttribute("l",0));
                // type is 1 for normal contact, 4 for mobile phone
                // TODO - use this value
                //int type = decimalFromString(contact.getAttribute("t",0));

                MSN::Passport passport(contact_name+"@"+domain_name);

                this->myNotificationServer()->externalCallbacks.removedListEntry(this, list_number, passport);
            }
        }
    }
    
    void NotificationServerConnection::handle_BLP(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        this->myNotificationServer()->externalCallbacks.gotBLP(this, args[3][0]);
    }

    void NotificationServerConnection::handle_CHG(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        this->myNotificationServer()->externalCallbacks.changedStatus(this, buddyStatusFromString(args[2]));
    }

    void NotificationServerConnection::handle_CHL(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        std::ostringstream buf_;
        buf_ << "QRY " << this->trID++ << " " << szClientID << " 32\r\n";
        if (write(buf_) != buf_.str().size())
            return;

        char b[33];
        memset(&b,0,33);
        DoMSNP11Challenge(args[2].c_str(),b);
        // send the md5
        std::string a(b);
        write(a, false);
    }

    void NotificationServerConnection::handle_ILN(std::vector<std::string> & args)
    {
        this->assertConnectionStateIs(NS_CONNECTED);
        if(args.size() > 7)
            this->myNotificationServer()->externalCallbacks.buddyChangedStatus(this, args[3], decodeURL(args[5]), buddyStatusFromString(args[2]), decimalFromString(args[6]), decodeURL(args[7]));
        else
            this->myNotificationServer()->externalCallbacks.buddyChangedStatus(this, args[3], decodeURL(args[5]), buddyStatusFromString(args[2]), decimalFromString(args[6]), "");
    }

    void NotificationServerConnection::handle_NLN(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        if(args.size() > 6)
            this->myNotificationServer()->externalCallbacks.buddyChangedStatus(this, args[2], decodeURL(args[4]), buddyStatusFromString(args[1]), decimalFromString(args[5]), decodeURL(args[6]));
        else
            this->myNotificationServer()->externalCallbacks.buddyChangedStatus(this, args[2], decodeURL(args[4]), buddyStatusFromString(args[1]), decimalFromString(args[5]), "");
    }
    
    void NotificationServerConnection::handle_FLN(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        this->myNotificationServer()->externalCallbacks.buddyOffline(this, args[1]);
    }    
 
    void NotificationServerConnection::handle_UBN(std::vector<std::string> & args)
    {
        // TODO - UBN is a way to exchange data through notification server
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        int msglen;
        std::string msg;
        
        msglen = decimalFromString(args[3]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);
    }

    void NotificationServerConnection::handle_FQY(std::vector<std::string> & args)
    {
        // TODO - I dont know what it means yet
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        int msglen;
        std::string msg;
        msglen = decimalFromString(args[2]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);
    }

    void NotificationServerConnection::callback_URL(std::vector<std::string> & args, int trid, void *data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        char b[33];
        MSN::hotmailInfo info;
        info.rru = args[2];
        info.url = args[3];
        info.id = args[4];
        info.sl = toStr(time(NULL) - decimalFromString(login_time));
        info.MSPAuth = MSPAuth;
        info.sid = sid;
        info.kv = kv;

        // calculate creds
        std::string creds_tmp = MSPAuth + info.sl + this->auth.password;
        memset(&b,0,33);

        md5_state_t state;
        md5_byte_t digest[16];
        int di;

        md5_init(&state);
        md5_append(&state, (const md5_byte_t *)creds_tmp.c_str(), creds_tmp.size());
        md5_finish(&state, digest);
        // convert to string
        for (di = 0; di < 16; ++di)
            sprintf(&b[2*di], "%02x", digest[di]);

        std::string creds(b);
        info.creds = creds;

        this->myNotificationServer()->externalCallbacks.gotInboxUrl(this, info);
    }
    
    void NotificationServerConnection::handle_MSG(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

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
        contentType = headers["Content-Type"];
        
        if ((tmp = contentType.find("; charset")) != std::string::npos)
            contentType = contentType.substr(0, tmp);
        
        std::map<std::string, void (NotificationServerConnection::*)(std::vector<std::string> &, std::string, std::string)>::iterator i = messageHandlers.find(contentType);
        if (i != messageHandlers.end())
            (this->*(messageHandlers[contentType]))(args, mime, body);
    }
    
    void NotificationServerConnection::message_initial_email_notification(std::vector<std::string> & args, std::string mime, std::string body)
    {
        std::string unreadInbox;
        std::string unreadFolder;
        int unreadInboxCount = 0, unreadFolderCount = 0;
        
        // Initial email notifications body is a set of MIME headers        
        Message::Headers headers = Message::Headers(body);
        
        unreadInbox = headers["Inbox-Unread"];
        unreadFolder = headers["Folders-Unread"];
        if (! unreadInbox.empty())
            unreadInboxCount = decimalFromString(unreadInbox);
        
        if (! unreadFolder.empty())
            unreadFolderCount = decimalFromString(unreadFolder);
        
//        this->myNotificationServer()->externalCallbacks.gotInitialEmailNotification(this, unreadInboxCount, unreadFolderCount);
    }

    void NotificationServerConnection::message_email_notification(std::vector<std::string> & args, std::string mime, std::string body)
    {
        // New email notifications body is a set of MIME headers
        Message::Headers headers = Message::Headers(body);
        
        std::string from = headers["From-Addr"];
        std::string subject = headers["Subject"];
        
        this->myNotificationServer()->externalCallbacks.gotNewEmailNotification(this, from, subject);
    }

    void NotificationServerConnection::message_msmsgsprofile(std::vector<std::string> & args, std::string mime, std::string body)
    {
        direct_connection=false;
        Message::Headers headers = Message::Headers(mime);
        server_reported_ip = headers["ClientIP"];
        server_reported_port = headers["ClientPort"];
        login_time = headers["LoginTime"];
        MSPAuth = headers["MSPAuth"];
        sid = headers["sid"];
        kv = headers["kv"];

        if (login_time.empty()) //IN MSNP9  there is no logintime it seems, so set it manualy
        {
            time_t actualTime;
            std::stringstream os;
            time(&actualTime);
            os << actualTime;
            login_time = os.str();
        }

        this->myNotificationServer()->externalCallbacks.gotNewConnection(this);
        // TODO - test portability to windows and mac, probably solved by ifdefs,
        // or with an external callback to user application

        // search on local machine the ip reported by the server
    /*  int s = socket (PF_INET, SOCK_STREAM, 0);
        for (int i=1;;i++)
        {
            struct ifreq ifr;
            struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
            char *ip;

            ifr.ifr_ifindex = i;
            if (ioctl (s, SIOCGIFNAME, &ifr) < 0)
              break;

            if (ioctl (s, SIOCGIFADDR, &ifr) < 0)
              continue;

            ip = inet_ntoa (sin->sin_addr);
            std::string ip2(ip);
            if(ip2==server_reported_ip)
                direct_connection=true;
        }
        close (s);
    */  
    }

    void NotificationServerConnection::message_initialmdatanotification(std::vector<std::string> & args, std::string mime, std::string body)
    {
        Message::Headers headers = Message::Headers(body);

        std::string maildata = headers["Mail-Data"];
        XMLNode domTree = XMLNode::parseString( maildata.c_str() );

        //Mail-Data: <MD><E><I>2</I><IU>1</IU><O>1</O><OU>0</OU></E><Q><QTM>409600</QTM><QNM>204800</QNM></Q></MD>
        //Mail-Data: <MD><E><I>3</I><IU>2</IU><O>2</O><OU>0</OU></E><Q><QTM>409600</QTM><QNM>204800</QNM></Q></MD>
        //Mail-Data: <MD><E><I>3</I><IU>1</IU><O>2</O><OU>0</OU></E><Q><QTM>409600</QTM><QNM>204800</QNM></Q></MD>
        //empty inbox
        //Mail-Data: <MD><E><I>0</I><IU>0</IU><O>5</O><OU>0</OU></E><Q><QTM>409600</QTM><QNM>204800</QNM></Q></MD>
        int emails = domTree.nChildNode("E");

        if (emails)
        {
            XMLNode curr_element = domTree.getChildNode("E",0);
            int inbox_msgs = decimalFromString(curr_element.getChildNode("I").getText());
            int inbox_unread = decimalFromString(curr_element.getChildNode("IU").getText());
            int other_folders = decimalFromString(curr_element.getChildNode("O").getText());
            int other_folders_unread = decimalFromString(curr_element.getChildNode("OU").getText());

            this->myNotificationServer()->externalCallbacks. gotInitialEmailNotification(this, inbox_msgs, inbox_unread, other_folders, other_folders_unread);
        }
        // try to get OIM information
        message_oimnotification(args, mime, body);
    }

    void NotificationServerConnection::message_oimnotification(std::vector<std::string> & args, std::string mime, std::string body)
    {
        Message::Headers headers = Message::Headers(body);

        std::string maildata = headers["Mail-Data"];

        if(maildata == "too-large")
        {
            // more than 25 OIM's
            // request OIM list through soap
            Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

            soapConnection->getMailData();
            return;
        }
        // process maildata
        gotMailData(maildata);
    }

    void NotificationServerConnection::gotSoapMailData(Soap & soapConnection, std::string maildata)
    {
        gotMailData(maildata);
    }

    void NotificationServerConnection::gotMailData(std::string maildata)
    {
        std::vector<eachOIM> messages;
        XMLNode domTree = XMLNode::parseString( maildata.c_str() );

        int oims = domTree.nChildNode("M");

        if (oims)
        {
            for(int i=0;i<oims;i++)
            {
                eachOIM temp_oim;
                XMLNode curr_element = domTree.getChildNode("M",i);
                temp_oim.from = curr_element.getChildNode("E").getText();
                temp_oim.id = curr_element.getChildNode("I").getText();
                temp_oim.fromFN = curr_element.getChildNode("N").getText();

                std::vector<std::string> friendlyName;
                // if we do not have '?', it is just the email 
                if(temp_oim.fromFN.find("?") != std::string::npos)
                {
                    friendlyName=splitString(temp_oim.fromFN,"?");
                    // TODO - handle the encoding (friendlyName[1])
                    if(friendlyName[2]=="B")
                    {
                        temp_oim.fromFN=b64_decode(friendlyName[3].c_str());
                    }
                    if(friendlyName[2]=="Q")
                    {
                        // Quoted-Printable, is similar to URL encoding, 
                        // but uses "=" instead of "%".
                        std::string change = friendlyName[3];
                        // changes the = by %
                        for(unsigned int a=0;a<change.length();a++)
                            if(change[a]=='=') change[a]='%';

                        temp_oim.fromFN=decodeURL(change);
                    }
                }
                messages.push_back(temp_oim);
            }
            this->myNotificationServer()->externalCallbacks.gotOIMList(this, messages);
        }
        domTree.deleteNodeContent('Y');
    }
 
    void NotificationServerConnection::handle_RNG(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        SwitchboardServerConnection::AuthData auth = SwitchboardServerConnection::AuthData(this->auth.username,
                                                                                           args[1],
                                                                                           args[4]);
        SwitchboardServerConnection *newSBconn = new SwitchboardServerConnection(auth, *this);
        this->addSwitchboardConnection(newSBconn);  
        std::pair<std::string, int> server_address = splitServerAddress(args[2]);  
        newSBconn->connect(server_address.first, server_address.second);        
    }
    
    void NotificationServerConnection::handle_PRP(std::vector<std::string> & args)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        if(this->_connectionState == NS_SYNCHRONISING)
        {
            this->myNotificationServer()->externalCallbacks.gotFriendlyName(this, decodeURL(args[3]));
            this->myDisplayName = decodeURL(args[3]);
            this->myNotificationServer()->externalCallbacks.connectionReady(this);
            // the initial process ends here
            this->setConnectionState(NS_CONNECTED);
            return;
        }
        if ( args[2] == "MFN") //when you set manually your FriendlyName
        {
                this->myNotificationServer()->externalCallbacks.gotFriendlyName(this, decodeURL(args[3]));
            this->myDisplayName = decodeURL(args[3]);

        }// TODO - Implement other PRP commands: MBE WWE
    }

    void NotificationServerConnection::handle_GCF(std::vector<std::string> & args)
    {
        int msglen;
        std::string msg;
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        // we do not use it so far. throw it away
        msglen = decimalFromString(args[2]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);
    }

    void NotificationServerConnection::handle_ADL(std::vector<std::string> & args)
    {
        int msglen;
        std::string msg;
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        if(args[2] =="OK" && (this->_connectionState == NS_SYNCHRONISING))
        {
            if(adl_packets.empty())
            {
                // no more adl packets to send!
                // now you need the change the nickname and set the status
                // to complete the initial connection process
                // it is not possible to use setFriendlyName
                // because we cannot send soap requests at this time
                std::ostringstream buf_;
                if(this->myDisplayName.empty())
                this->myDisplayName = myPassport;
                if(server_email_verified != "0")
                {
                    // our email is verified
                    buf_ << "PRP " << this->trID++ << " MFN "  << encodeURL(this->myDisplayName) << "\r\n";
                    write(buf_);
                }
                else
                {
                    // not verified, so we cant change our displayName
                    this->myNotificationServer()->externalCallbacks.connectionReady(this);
                    // the initial process ends here
                    this->setConnectionState(NS_CONNECTED);
                }
                return;
            }
            else
            {
                // send each adl packet at a time
                std::string adl_payload = adl_packets.front();
                adl_packets.pop_front();
                std::ostringstream buf_;
                buf_ << "ADL " << this->trID++ << " " << adl_payload.length() << "\r\n";
                buf_ << adl_payload;
                if (write(buf_) != buf_.str().size())
                    return;
            }
        }
        // I reach here when ADL has payload
        msglen = decimalFromString(args[2]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);
        // ADL 0 70
        // <ml><d n="domain.com"><c n="foo.bar" t="1" l="8" f="fName" /></d></ml>
        // 

        XMLNode adl_data = XMLNode::parseString( msg.c_str() );

        int nDomains = adl_data.nChildNode("d");
        for(int i=0; i<nDomains; i++)
        {
            XMLNode domain = adl_data.getChildNode("d",i);
            std::string domain_name = domain.getAttribute("n",0);
            int nContacts =  domain.nChildNode("c");
            for(int j=0; j< nContacts; j++)
            {
                XMLNode contact = domain.getChildNode("c",j);
                std::string contact_name = contact.getAttribute("n",0);
                std::string fname = contact.getAttribute("f",0);
                MSN::ContactList list_number = (MSN::ContactList) decimalFromString(contact.getAttribute("l",0));
                // type is 1 for wlm contacts, 4 for mobile phone, 32 for yahoo? (or any kind of email contact)
                int type = decimalFromString(contact.getAttribute("t",0));
                if(type == 32)
                    return;

                MSN::Passport passport(contact_name+"@"+domain_name);

                this->myNotificationServer()->externalCallbacks.addedListEntry(this, list_number, passport, fname);
            }
        }
    }

    void NotificationServerConnection::handle_UBX(std::vector<std::string> & args)
    {
        int msglen;
        personalInfo pInfo;
        std::string msg,media,psm;
        MSN::Passport fromPassport = args[1];
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        msglen = decimalFromString(args[3]);
        msg = this->readBuffer.substr(0, msglen);
        this->readBuffer = this->readBuffer.substr(msglen);

        // some buggy clients send no data in UBX command
        if( msg.length() < 10 ) return;

        XMLNode ubx_data = XMLNode::parseString( msg.c_str() );

        const char *a = ubx_data.getChildNode("PSM").getText();
        if(a)
        {
            psm = a;
            pInfo.PSM = psm;
        }

        const char *m = ubx_data.getChildNode("CurrentMedia").getText();
        if(m)
        {
            media = m;
            // the splitString will drop the first NULL position.
            // we need it to keep the order of the following fields.
            std::vector<std::string> media1 = splitString(media, "\\0");
            if(media1.size()>=4) // at least 4 fields. Type, Enabled, format, data
            { // if we have some field, so ...
                int i=0;
                if (media.find("\\0")==0)
                { // if starts with \0 there is no
                  // App field. It is optional.
                    pInfo.mediaApp = "";
                }
                else
                {
                    pInfo.mediaApp = media1[i++];
                }
                pInfo.mediaType = media1[i++];
                pInfo.mediaIsEnabled = decimalFromString(media1[i++]);

                if(pInfo.mediaIsEnabled)
                {
                    pInfo.mediaFormat = media1[i++];
                    for(unsigned int b=i; b < media1.size(); b++)
                    {
                        pInfo.mediaLines.push_back(media1[i++]);
                    }
                }
            }
        }
        this->myNotificationServer()->externalCallbacks.buddyChangedPersonalInfo(this, fromPassport, pInfo);
    }
 
    void NotificationServerConnection::setState(BuddyStatus state, uint clientID)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        std::ostringstream buf_;
        std::string xml;

        if(msnobj.getMSNObjectXMLByType(3,xml))
                buf_ << "CHG " << this->trID++ << " " << 
                buddyStatusToString(state) << " "<< unsignedToStr(clientID) << " " << encodeURL(xml) << "\r\n";
        else
                buf_ << "CHG " << this->trID++ << " " << buddyStatusToString(state) << " " << unsignedToStr(clientID) << "\r\n";
        write(buf_);
    }
    
    void NotificationServerConnection::setBLP(char setting)
    {
        if (setting != 'A' || setting != 'B')
            return;

        if(this->_connectionState == NS_CONNECTED)
        {
            std::ostringstream buf_;
            this->bplSetting = setting;
            buf_ << "BLP " << this->trID++ << " " << setting << "L\r\n";
            write(buf_);
        }
        else
        {
            this->bplSetting = setting;
        }
    }

    void NotificationServerConnection::setFriendlyName(std::string friendlyName, bool updateServer) throw (std::runtime_error)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        if(friendlyName.empty())
            return;
        if (friendlyName.size() > 387)
            throw std::runtime_error("Friendly name too long!");

        if(updateServer)
        {
            // update nickname on server
            Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
            soapConnection->changeDisplayName(friendlyName);
        }
        else
        {
            this->myDisplayName = friendlyName;
            std::ostringstream buf_;
            buf_ << "PRP " << this->trID++ << " MFN "  << encodeURL(friendlyName) << "\r\n";
            write(buf_);
        }
    }

    void NotificationServerConnection::setPersonalStatus(personalInfo pInfo)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        std::string tempMedia;
        XMLNode data = XMLNode::createXMLTopNode("Data");
        XMLNode PSM = XMLNode::createXMLTopNode("PSM");
        XMLNode CurrentMedia = XMLNode::createXMLTopNode("CurrentMedia");
        XMLNode MachineGuid = XMLNode::createXMLTopNode("MachineGuid");

        PSM.addText( pInfo.PSM.c_str() );
        if(pInfo.mediaIsEnabled)
        {
            tempMedia = pInfo.mediaApp +"\\0"+ 
                    pInfo.mediaType+"\\0"+
                    toStr(pInfo.mediaIsEnabled)+"\\0"+
                    pInfo.mediaFormat +"\\0";

            std::vector<std::string>::iterator i = pInfo.mediaLines.begin();
            for(;i!=pInfo.mediaLines.end();i++)
            {
                tempMedia+=(*i);
                tempMedia+="\\0";
            }
        }
        CurrentMedia.addText( tempMedia.c_str() );

        data.addChild(PSM);
        data.addChild(CurrentMedia);

        char *payload1=data.createXMLString(false);

        std::string payload(payload1);
        free(payload1);
        std::ostringstream buf_;  
        buf_ << "UUX " << this->trID++ << " " << payload.length() << "\r\n";
        buf_ << payload;
        write(buf_);
    }

    void NotificationServerConnection::blockContact(Passport buddyName)
    {
        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->removeContactFromList(buddyName,LST_AL);

        soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->addContactToList(buddyName,LST_BL);
    }

    void NotificationServerConnection::unblockContact(Passport buddyName)
    {
        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->removeContactFromList(buddyName,LST_BL);

        soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->addContactToList(buddyName,LST_AL);
    }

    void NotificationServerConnection::addToAddressBook(Passport buddyName, std::string displayName)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->addContactToAddressBook(buddyName, displayName);
    }

    void NotificationServerConnection::enableContactOnAddressBook(std::string contactId, std::string passport)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->enableContactOnAddressBook(contactId, passport, this->myDisplayName);
    }

    void NotificationServerConnection::disableContactOnAddressBook(std::string contactId, std::string passport)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->disableContactFromAddressBook(contactId,passport);
    }

    void NotificationServerConnection::delFromAddressBook(std::string contactId, std::string passport)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        std::vector<std::string> passport2 = splitString(passport, "@");
        std::string user = passport2[0];
        std::string domain = passport2[1];

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->delContactFromAddressBook(contactId,passport);
    }

    void NotificationServerConnection::addToList(MSN::ContactList list, Passport buddyName)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->addContactToList(buddyName,list);
    }
    
    void NotificationServerConnection::removeFromList(MSN::ContactList list, Passport buddyName)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->removeContactFromList(buddyName,list);
    }
    
    void NotificationServerConnection::addToGroup(std::string groupId, std::string contactId)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->addContactToGroup(groupId, contactId);
    }
    
    void NotificationServerConnection::removeFromGroup(std::string groupId, std::string contactId)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->delContactFromGroup(groupId, contactId);
    }
    
    void NotificationServerConnection::addGroup(std::string groupName)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->addGroup(groupName);
    }
    
    void NotificationServerConnection::removeGroup(std::string groupID)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->delGroup(groupID);
    }
    
    void NotificationServerConnection::renameGroup(std::string groupID, std::string newGroupName)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->renameGroup(groupID, newGroupName);
    }
    
    void NotificationServerConnection::synchronizeContactList(std::string lastChange)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        this->assertConnectionStateIsNot(NS_SYNCHRONISING);

        // we are synchronizing through soap requests now
        this->setConnectionState(NS_SYNCHRONISING);

        listInfo = new ListSyncInfo(lastChange);
        if(!listInfo)
            return; // TODO - raise an error

        if(!lastChange.length()) lastChange = "0";

        listInfo->lastChange = lastChange;

        Soap *soapConnection;
        soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->getLists(listInfo);
    }

    void NotificationServerConnection::gotLists(Soap &soapConnection)
    {
        if(!listInfo)
            return; // TODO - raise an error

        Soap *soapConnection1;
        soapConnection1 = new Soap(*this, this->sitesToAuthList);

        // ask for Address book
        soapConnection1->getAddressBook(this->listInfo);
    }

    void NotificationServerConnection::gotAddressBook(Soap &soapConnection)
    {
        // TODO - sorry, I dont have choice. I need this here because the initial setFriendlyName
        // is called by handle_ADL(), which does not have access to info variable.
        this->myDisplayName = listInfo->myDisplayName;

        std::ostringstream buf_;

        // TODO - see what is the user choice: BL or AL
        // A value of 'AL' indicates that users that are neither on the client's Allow List or Buddy List will be allowed to see the client's online status and open a switchboard session with the client. A value of 'BL' indicates that these users will see the client as offline and will not be allowed to open a switchboard session.
        // http://msnpiki.msnfanatic.com/index.php/Command:BLP

        buf_ << "BLP " << this->trID << " " << this->bplSetting << "L\r\n";
        if (write(buf_) != buf_.str().size())
            return;

        this->addCallback(&NotificationServerConnection::callback_initialBPL, this->trID++, (void *)NULL);
    }
    
    void NotificationServerConnection::callback_initialBPL(std::vector<std::string> & args, int trid, void *data)
    {
        this->assertConnectionStateIs(NS_SYNCHRONISING);
        this->removeCallback(trid);

        this->myNotificationServer()->externalCallbacks.gotBuddyListInfo(this, this->listInfo);
        delete this->listInfo;
    }

    void NotificationServerConnection::completeConnection(std::map<std::string, int > & allContacts, void *info)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);

        // FIXME - handle the errors
        std::map<std::string, std::vector<std::string> > domains;

        std::string tempADL;

        // contains which number is. (i.e. 1 for FL, 2 for AL, 4 for BL) or the sum
        std::map<std::string, int> tempList;

        std::map<std::string,int>::iterator i;
        for (i = allContacts.begin(); i != allContacts.end(); i++)
        {
             std::vector<std::string> parts = splitString((*i).first,"@");
             if(tempList[(*i).first]==0)
                domains[parts[1]].push_back(parts[0]);

             // it does not allow LST_AL and LST_BL, as the server will refuse with a 241 error
             int privacyListMask = MSN::LST_AL | MSN::LST_BL;
             if ( ((*i).second & privacyListMask) == privacyListMask )
                tempList[(*i).first] = (*i).second & ~MSN::LST_AL;
             else
                tempList[(*i).first] = (*i).second;
        }

        // deleting buddy information
        std::map<std::string, MSN::Buddy *>::iterator d = listInfo->contactList.begin();
        for(d; d != listInfo->contactList.end(); d++)
        {
            delete (*d).second;
        }

        // adding domains and users to xml:
        // The max payload of ADL command is about 7500 bytes.
        // so the code below should do that.
        // not using xmlParser due to the complex algorithm
        // TODO - What to do when there are no contacts?
        std::map<std::string, std::vector<std::string> >::iterator cur = domains.begin();

        tempADL = "";
        // for each domain
        for(; cur != domains.end(); cur++)
        {
            do
            {
                tempADL += "<d n=\"" + (*cur).first + "\">"; 
                // for each user
                while(domains[(*cur).first].size()!=0)
                {
                    std::string a((*cur).second[0]+"@"+(*cur).first);
                    tempADL += "<c n=\"" + (*cur).second[0] + "\" l=\""+toStr(tempList[a]) +"\" t=\"1\"/>";
                    (*cur).second.erase((*cur).second.begin());

                    if (tempADL.length()>7400)
                        break;
                }
                tempADL += "</d>";

                if (tempADL.length()>7400)
                {
                    adl_packets.push_back("<ml l=\"1\">" + tempADL + "</ml>" );
                    tempADL = "";
                }
            } while(domains[(*cur).first].size()!=0);
        }
        adl_packets.push_back("<ml l=\"1\">" + tempADL + "</ml>" );

        // send the first one
        std::string adl_payload = adl_packets.front();
        adl_packets.pop_front();

        std::ostringstream buf_;
        buf_ << "ADL " << this->trID++ << " " << adl_payload.length() << "\r\n";
        buf_ << adl_payload;
        if (write(buf_) != buf_.str().size())
            return;
    }
 

    void NotificationServerConnection::sendPing()
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        std::string a("PNG\r\n");
        write(a);
    }

    void NotificationServerConnection::requestSwitchboardConnection(const void *tag)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        SwitchboardServerConnection::AuthData *auth = new SwitchboardServerConnection::AuthData(this->auth.username, tag);
        std::ostringstream buf_;
        buf_ << "XFR " << this->trID << " SB\r\n";
        if (write(buf_) != buf_.str().size())
            return;

        this->addCallback(&NotificationServerConnection::callback_TransferToSwitchboard, this->trID++, (void *)auth);
    }
    
    template <class _Tp>
    class _sameUserName
    {
        Buddy buddy;
public:
        _sameUserName(const _Tp &__u) : buddy(__u) {};
        bool operator()(const _Tp &__x) { return __x.userName == buddy.userName; }
    };
    
    void NotificationServerConnection::socketConnectionCompleted()
    {
        this->assertConnectionStateIs(NS_CONNECTING);
        this->setConnectionState(NS_CONNECTED);
        
        Connection::socketConnectionCompleted();
        
        // If an error occurs in Connection::socketConnectionCompleted, we
        // will be disconnected before we get here.
        if (this->connectionState() != NS_DISCONNECTED)
        {
            this->myNotificationServer()->externalCallbacks.unregisterSocket(this->sock);
            this->myNotificationServer()->externalCallbacks.registerSocket(this->sock, 1, 0, false);
        }
    }
    
    void NotificationServerConnection::connect(const std::string & hostname, unsigned int port)
    {
        this->assertConnectionStateIs(NS_DISCONNECTED);
        connectinfo *info = new connectinfo(this->auth.username, this->auth.password);
        this->info = info;
        
        if ((this->sock = this->myNotificationServer()->externalCallbacks.connectToServer(hostname, port, &this->connected)) == NULL)
        {
            this->myNotificationServer()->externalCallbacks.showError(this, "Could not connect to MSN server");
            this->myNotificationServer()->externalCallbacks.closingConnection(this);
            return;
        }
        this->setConnectionState(NS_CONNECTING);
        this->myNotificationServer()->externalCallbacks.registerSocket(this->sock, 0, 1, false);
        if (this->connected)
            this->socketConnectionCompleted();
        
        std::ostringstream buf_;
        buf_ << "VER " << this->trID << " MSNP15 CVR0\r\n";
        if (this->write(buf_) != buf_.str().size())
            return;
        
        this->addCallback(&NotificationServerConnection::callback_NegotiateCVR, this->trID++, (void *)info);
    }
    
    void NotificationServerConnection::connect(const std::string & hostname, unsigned int port, const Passport & username, const std::string & password)
    {
        this->auth.username = username;
        this->auth.password = password;
        this->connect(hostname, port);
    }
    
    void NotificationServerConnection::disconnect()
    {
        if (this->connectionState() == NS_DISCONNECTED)
            return;

        std::vector<SwitchboardServerConnection *> list = _switchboardConnections;
        std::vector<SwitchboardServerConnection *>::iterator i = list.begin();
        for (; i != list.end(); ++i)
        {
            delete *i;
        }
        std::vector<Soap *> list2 = _SoapConnections;
        std::vector<Soap *>::iterator d = list2.begin();

        for (; d != list2.end(); ++d)
        {
             delete *d;
        }

        this->callbacks.clear();
        this->sitesToAuthList.erase(sitesToAuthList.begin(), sitesToAuthList.end());
        SentQueuedOIMs.erase(SentQueuedOIMs.begin(), SentQueuedOIMs.end());
        this->setConnectionState(NS_DISCONNECTED);
        this->myNotificationServer()->externalCallbacks.closingConnection(this);
        Connection::disconnect();
    }
    
    void NotificationServerConnection::disconnectForTransfer()
    {
        this->assertConnectionStateIsNot(NS_DISCONNECTED);
        this->myNotificationServer()->externalCallbacks.unregisterSocket(this->sock);
        this->myNotificationServer()->externalCallbacks.closeSocket(this->sock);
        this->setConnectionState(NS_DISCONNECTED);
    }
    
    void NotificationServerConnection::handleIncomingData()
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        while (this->isWholeLineAvailable())
        {
            std::vector<std::string> args = this->getLine();
            if(!args.size()) continue;
            if (args[0] == "MSG" || args[0] == "NOT" || 
                args[0] == "IPG" || args[0] == "GCF" || 
                args[0] == "UBX" || args[0] == "ADL" ||
                args[0] == "RML")
            {
                int dataLength;
                if (args[0] == "MSG" || args[0] == "UBX")
                    dataLength = decimalFromString(args[3]);
                else if(args[0] == "GCF" || args[0] == "ADL" || args[0] == "RML")
                    dataLength = decimalFromString(args[2]);
                else
                    dataLength = decimalFromString(args[1]);

                if (this->readBuffer.find("\r\n") + 2 + dataLength > this->readBuffer.size())
                    return;
            }
            this->readBuffer = this->readBuffer.substr(this->readBuffer.find("\r\n") + 2);
            int trid = 0;
            
            if (args.size() >= 6 && args[0] == "XFR" && args[2] == "NS")
            {
                // XFR TrID NS NotificationServerIP:Port 0 ThisServerIP:Port
                //  0    1  2             3              4        5    
                this->callbacks.clear(); // delete the callback data
                
                this->disconnectForTransfer();
                
                std::pair<std::string, int> server_address = splitServerAddress(args[3]);
                this->connect(server_address.first, server_address.second);
                return;
            }
            
            if (args.size() >= 7 && args[0] == "RNG")
            {
                // RNG SessionID SwitchboardServerIP:Port CKI AuthString InvitingUser InvitingDisplayName
                // 0      1                 2              3       4           5             6
                this->handle_RNG(args);
                return;
            }

            if (args.size() >= 2 && args[0] == "QNG")
            {
                // QNG seconds
                //  0   1
                
                // ping response, ignore
                return;
            }
            
            if ((args.size() >= 3 && args[0] == "LST" ) ||
                (args.size() >= 2 && (args[0] == "GTC" )) ||
                (args.size() >= 3 && (args[0] == "BPR" || args[0] == "LSG" ))
           )
            {
                // LST N=UserName F=FriendlyName C=GUID param groupID
                //  0      1          2          3      4      5
                //
                // or
                // (GTC|BLP) [TrID] [ListVersion] Setting
                //     0        1        2          4
                
                if (this->synctrid)
                {
                    trid = this->synctrid;
                }
                else
                {
                    trid = decimalFromString(args[1]);
                }
            } 
            else if (args.size() > 1)
            {
                try
                {
                    trid = decimalFromString(args[1]);
                }
                catch (...)
                {
                }
            }
            
            if (!this->callbacks.empty() && trid >= 0)
            {
                if (this->callbacks.find(trid) != this->callbacks.end())
                {
                    (this->*(this->callbacks[trid].first))(args, trid, this->callbacks[trid].second);
                    continue;
                }
            }
            
            if (isdigit(args[0][0]))
                this->showError(decimalFromString(args[0]));
            else
                this->dispatchCommand(args);
        }
    }
    
    void NotificationServerConnection::callback_NegotiateCVR(std::vector<std::string> & args, int trid, void *data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        connectinfo * info = (connectinfo *) data;
        this->removeCallback(trid);
        if (args.size() >= 3 && args[0] != "VER" || args[2] != "MSNP15") // if either *differs*...
        {
            this->myNotificationServer()->externalCallbacks.showError(NULL, "Protocol negotiation failed");
            this->disconnect();
            return;
        }

        std::ostringstream buf_;
        buf_ << "CVR " << this->trID << " 0x0409 winnt 5.1 i386 MSG80BETA 8.1.0178.00 MSMSGS " << info->username << "\r\n";
        if (this->write(buf_) != buf_.str().size())
            return;
        this->addCallback(&NotificationServerConnection::callback_RequestUSR, this->trID++, (void *) data);
    }
    
    void NotificationServerConnection::callback_TransferToSwitchboard(std::vector<std::string> & args, int trid, void *data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        SwitchboardServerConnection::AuthData *auth = static_cast<SwitchboardServerConnection::AuthData *>(data);
        this->removeCallback(trid);
        
        if (args[0] != "XFR")
        {
            this->showError(decimalFromString(args[0]));
            this->disconnect();
            delete auth;
            return;
        }
        
        auth->cookie = args[5];
        auth->sessionID = "";
        
        SwitchboardServerConnection *newconn = new SwitchboardServerConnection(*auth, *this);
        
        this->addSwitchboardConnection(newconn);
        std::pair<std::string, int> server_address = splitServerAddress(args[3]);
        newconn->connect(server_address.first, server_address.second);
        
        delete auth;
    }
    
    void NotificationServerConnection::callback_RequestUSR(std::vector<std::string> & args, int trid, void *data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        connectinfo *info = (connectinfo *)data;
        this->removeCallback(trid);
        
        if (args.size() > 1 && args[0] != "CVR") // if*differs*...
        {
            this->myNotificationServer()->externalCallbacks.showError(NULL, "Protocol negotiation failed");
            this->disconnect();
            return;
        }
        
        std::ostringstream buf_;
        buf_ << "USR " << this->trID << " SSO I " << info->username << "\r\n";
        if (this->write(buf_) != buf_.str().size())
            return;
        
        this->addCallback(&NotificationServerConnection::callback_PassportAuthentication, this->trID++, (void *) data);
    }    
    
    void NotificationServerConnection::callback_PassportAuthentication(std::vector<std::string> & args, int trid, void * data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        connectinfo * info;
        
        info=(connectinfo *)data;
        this->removeCallback(trid);

        if (isdigit(args[0][0]))
        {
            this->showError(decimalFromString(args[0]));
            this->disconnect();
            return;
        }
        
        if (args.size() >= 4 && args[4].empty()) {
            this->disconnect();
            return;
        }
        
        this->myNotificationServer()->externalCallbacks.getSecureHTTPProxy();

        Soap *soapConnection = new Soap(*this);

        this->mdi = args[5];
        soapConnection->setMBI(args[4]);
        soapConnection->getTickets(info->username,info->password,args[4]);
        delete info;
        info=NULL;
    }

    void NotificationServerConnection::gotTickets(Soap & soapConnection, std::vector<MSN::Soap::sitesToAuth> sitesToAuthList)
    {
        std::ostringstream buf_;
        this->sitesToAuthList = sitesToAuthList;
        std::string token = sitesToAuthList[1].BinarySecurityToken;
        std::string binarysecret = sitesToAuthList[1].BinarySecret;
        this->token = token;

        buf_ << "USR " << this->trID << " SSO S " <<  token << " " << mdi_encrypt(binarysecret, mdi) << "\r\n";
        if (this->write(buf_) != buf_.str().size())
            return;
        this->addCallback(&NotificationServerConnection::callback_AuthenticationComplete, this->trID++, NULL);
    }
    
    void NotificationServerConnection::callback_AuthenticationComplete(std::vector<std::string> & args, int trid, void * data)
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        this->removeCallback(trid);

        if (isdigit(args[0][0]))
        {
            this->showError(decimalFromString(args[0]));
            this->disconnect();
            return;
        }
        server_email_verified = args[4];
    }   

    void NotificationServerConnection::get_oim(std::string id, bool markAsRead)
    {
        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

        soapConnection->getOIM(id,markAsRead);
    }

    void NotificationServerConnection::delete_oim(std::string id)
    {
        if(this->removingOIM)
        {
            DeletedQueuedOIMs.push_back(id);
            return;
        }
        this->removingOIM = true;
        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        soapConnection->deleteOIM(id);
    }

    void NotificationServerConnection::send_oim(Soap::OIM oim)
    {
        // do not generate two lockkeys at the same time
        if(this->generatingLockkey)
        {
            SentQueuedOIMs.push_back(oim);
            return;
        }
        Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
        SentQueuedOIMs.push_back(oim);
        this->generatingLockkey=true;
        soapConnection->generateLockkey(oim);
    }

    void NotificationServerConnection::gotOIM(Soap & soapConnection, bool success, std::string id, std::string message)
    {
        this->myNotificationServer()->externalCallbacks.gotOIM(this, success, id, message);
    }

    void NotificationServerConnection::gotOIMLockkey(Soap & soapConnection, std::string lockkey)
    {
        this->lockkey = lockkey;
        this->generatingLockkey = false;
        if(this->lockkey.empty())
        {
            std::vector<Soap::OIM>::iterator i = SentQueuedOIMs.begin();
            for(; i != SentQueuedOIMs.end(); i++)
            {
                this->myNotificationServer()->externalCallbacks.gotOIMSendConfirmation(this, false, (*i).id);
            }
            SentQueuedOIMs.erase(SentQueuedOIMs.begin(), SentQueuedOIMs.end());
            return;
        }
        sendQueuedOIMs();
    }

    void NotificationServerConnection::gotOIMDeleteConfirmation(Soap & soapConnection, std::string id, bool deleted)
    {
        this->myNotificationServer()->externalCallbacks.gotOIMDeleteConfirmation(this, deleted, id);
        if(this->DeletedQueuedOIMs.empty())
        {
            removingOIM = false;
            return;
        }
        else
        {
            Soap *soapConnection = new Soap(*this, this->sitesToAuthList);
            soapConnection->deleteOIM(DeletedQueuedOIMs.back());
            DeletedQueuedOIMs.pop_back();
        }
    }

    void NotificationServerConnection::gotOIMSendConfirmation(Soap & soapConnection, int id, bool sent)
    {
        if(!sent)
            this->lockkey.clear();

        this->myNotificationServer()->externalCallbacks.gotOIMSendConfirmation(this, sent, id);
    }

    void NotificationServerConnection::sendQueuedOIMs()
    {
        std::vector<Soap::OIM>::iterator i = SentQueuedOIMs.begin();
        for(; i != SentQueuedOIMs.end(); i++)
        {
            Soap *soapConnection = new Soap(*this, this->sitesToAuthList);

            soapConnection->sendOIM((*i), this->lockkey);
        }
        SentQueuedOIMs.erase(SentQueuedOIMs.begin(), SentQueuedOIMs.end());
    }

    bool NotificationServerConnection::change_DisplayPicture(std::string filename)
    {
        msnobj.delMSNObjectByType(3);
        if(!filename.empty())
            msnobj.addMSNObject(filename,3);

        return true;
    }

    void NotificationServerConnection::gotChangeDisplayNameConfirmation(Soap & soapConnection, std::string displayName, bool changed)
    {
        if(changed)
        {
            this->myDisplayName = displayName;
            // server update OK, now change to the current session
            std::ostringstream buf_;
            buf_ << "PRP " << this->trID++ << " MFN "  << encodeURL(displayName) << "\r\n";
            write(buf_);
        }
        // TODO - raise an error if not changed
    }

    void NotificationServerConnection::gotAddContactToGroupConfirmation(Soap & soapConnection, bool added, std::string newVersion, std::string groupId, std::string contactId)
    {
        this->myNotificationServer()->externalCallbacks.addedContactToGroup(this, added, groupId, contactId);
    }

    void NotificationServerConnection::gotDelContactFromGroupConfirmation(Soap & soapConnection, bool removed, std::string newVersion, std::string groupId, std::string contactId)
    {
        this->myNotificationServer()->externalCallbacks.removedContactFromGroup(this, removed, groupId, contactId);
    }

    void NotificationServerConnection::gotAddGroupConfirmation(Soap & soapConnection, bool added, std::string newVersion, std::string groupName, std::string groupId)
    {
        this->myNotificationServer()->externalCallbacks.addedGroup(this, added, groupName, groupId);
    }

    void NotificationServerConnection::gotDelGroupConfirmation(Soap & soapConnection, bool removed, std::string newVersion, std::string groupId)
    {
        this->myNotificationServer()->externalCallbacks.removedGroup(this, removed, groupId);
    }
    void NotificationServerConnection::gotRenameGroupConfirmation(Soap & soapConnection, bool renamed, std::string newVersion, std::string newGroupName, std::string groupId)
    {
        this->myNotificationServer()->externalCallbacks.renamedGroup(this, renamed, newGroupName, groupId);
    }

    void NotificationServerConnection::gotAddContactToAddressBookConfirmation(Soap & soapConnection, bool added, std::string newVersion, std::string passport, std::string displayName, std::string guid)
    {
        this->myNotificationServer()->externalCallbacks.addedContactToAddressBook(this, added, passport, displayName, guid);
        if(added)
        {
            std::vector<std::string> passport2 = splitString(passport, "@");
            std::string user = passport2[0];
            std::string domain = passport2[1];

            // TODO - use xmlParser
            std::string payload3("<ml><d n=\"" + domain  + "\"><c n=\"" + user+ "\" l=\"2\" t=\"1\"/></d></ml>");
            std::ostringstream buf_3;
            buf_3 << "ADL " << this->trID++ << " " << payload3.length() << "\r\n";
            buf_3 << payload3;
            write(buf_3);

            std::string payload2("<ml><d n=\"" + domain  + "\"><c n=\"" + user+ "\" l=\"1\" t=\"1\"/></d></ml>");
            std::ostringstream buf_2;
            buf_2 << "ADL " << this->trID++ << " " << payload2.length() << "\r\n";
            buf_2 << payload2;
            write(buf_2);

            // the official client sends FQY
            std::string payload4("<ml l=\"2\"><d n=\"" + domain  + "\"><c n=\"" + user+ "\"/></d></ml>");
            std::ostringstream buf_4;
            buf_4 << "FQY " << this->trID++ << " " << payload4.length() << "\r\n";
            buf_4 << payload4;
            write(buf_4);
        }
    }

    void NotificationServerConnection::gotDelContactFromAddressBookConfirmation(Soap & soapConnection, bool removed, std::string newVersion, std::string contactId, std::string passport)
    {
        this->myNotificationServer()->externalCallbacks.removedContactFromAddressBook(this, removed, contactId, passport);
        if(removed)
        {
            std::vector<std::string> passport2 = splitString(passport, "@");
            std::string user = passport2[0];
            std::string domain = passport2[1];

            // TODO - use xmlParser
            std::string payload2("<ml><d n=\"" + domain  + "\"><c n=\"" + user+ "\" l=\"1\" t=\"1\"/></d></ml>");
            std::ostringstream buf_2;
            buf_2 << "RML " << this->trID++ << " " << payload2.length() << "\r\n";
            buf_2 << payload2;
            write(buf_2);

        }
    }

    void NotificationServerConnection::gotEnableContactOnAddressBookConfirmation(Soap & soapConnection, bool disabled, std::string newVersion, std::string contactId, std::string passport)
    {
        this->myNotificationServer()->externalCallbacks.enabledContactOnAddressBook(this, disabled, contactId, passport);
        if(disabled)
        {
            std::vector<std::string> passport2 = splitString(passport, "@");
            std::string user = passport2[0];
            std::string domain = passport2[1];

            // TODO - use xmlParser
            std::string payload3("<ml><d n=\"" + domain  + "\"><c n=\"" + user+ "\" l=\"1\" t=\"1\"/></d></ml>");
            std::ostringstream buf_3;
            buf_3 << "ADL " << this->trID++ << " " << payload3.length() << "\r\n";
            buf_3 << payload3;
            write(buf_3);
        }
    }

    void NotificationServerConnection::gotDisableContactOnAddressBookConfirmation(Soap & soapConnection, bool disabled, std::string newVersion, std::string contactId, std::string passport)
    {
        this->myNotificationServer()->externalCallbacks.disabledContactOnAddressBook(this, disabled, contactId);
        if(disabled)
        {
            std::vector<std::string> passport2 = splitString(passport, "@");
            std::string user = passport2[0];
            std::string domain = passport2[1];

            // TODO - use xmlParser
            std::string payload2("<ml><d n=\"" + domain  + "\"><c n=\"" + user+ "\" l=\"1\" t=\"1\"/></d></ml>");
            std::ostringstream buf_2;
            buf_2 << "RML " << this->trID++ << " " << payload2.length() << "\r\n";
            buf_2 << payload2;
            write(buf_2);
        }
    }

    void NotificationServerConnection::gotAddContactToListConfirmation(Soap & soapConnection, bool added, std::string newVersion, std::string passport, MSN::ContactList list)
    {
        if(added)
        {
            std::vector<std::string> passport2 = splitString(passport, "@");
            std::string user = passport2[0];
            std::string domain = passport2[1];

            // TODO - use xmlParser
            std::string payload2("<ml><d n=\"" + domain  + "\"><c n=\"" + user+ "\" l=\""+toStr(list)+"\" t=\"1\"/></d></ml>");
            std::ostringstream buf_2;
            buf_2 << "ADL " << this->trID++ << " " << payload2.length() << "\r\n";
            buf_2 << payload2;
            write(buf_2);
            this->myNotificationServer()->externalCallbacks.addedListEntry(this, list, passport, "");
        }
    }
    void NotificationServerConnection::gotDelContactFromListConfirmation(Soap & soapConnection, bool deleted, std::string newVersion, std::string passport, MSN::ContactList list)
    {
        if(deleted)
        {
            std::vector<std::string> passport2 = splitString(passport, "@");
            std::string user = passport2[0];
            std::string domain = passport2[1];
            // TODO - use XMLParser
            std::string payload("<ml><d n=\""+ domain + "\"><c n=\"" + user + "\" l=\""+toStr(list)+"\" t=\"1\"/></d></ml>");

            std::ostringstream buf_;  
            buf_ << "RML " << this->trID++ << " " << payload.length() << "\r\n";
            buf_ << payload;
            write(buf_);
            this->myNotificationServer()->externalCallbacks.removedListEntry(this, list, passport);
        }
    }

    void NotificationServerConnection::setCapabilities(uint m_clientId)
    {
        this->m_clientId = m_clientId;
    }

    void NotificationServerConnection::getInboxUrl()
    {
        this->assertConnectionStateIsAtLeast(NS_CONNECTED);
        std::ostringstream buf_;  
        buf_ << "URL " << this->trID << " INBOX\r\n";
        write(buf_);
        this->addCallback(&NotificationServerConnection::callback_URL, this->trID++, NULL);
    }
}
