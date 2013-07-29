/*
 * soap.cpp
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



#include <notificationserver.h>
#include <errorcodes.h>
#include <externals.h>
#include <md5.h>
#include <util.h>
#include <soap.h>

#include <cctype>
#include <iostream>
#include <algorithm>
#include <string.h>

#include "xmlParser.h"

namespace MSN {
    std::map<int,std::string> Soap::actionDomains;
    std::map<int,std::string> Soap::actionPOSTURLs;
    std::map<int,std::string> Soap::actionURLs;

    Soap::Soap(NotificationServerConnection & _myNotificationServer) : 
        Connection(), 
        notificationServer(_myNotificationServer)
    {
        fillURLs();
    }

    Soap::Soap(NotificationServerConnection & _myNotificationServer, std::vector<sitesToAuth> _sitesToAuthList) : 
        Connection(), 
        notificationServer(_myNotificationServer), 
        sitesToAuthList(_sitesToAuthList)
    {
        fillURLs();
    }

    void Soap::requestSoapAction(soapAction action,std::string xml_body, std::string &xml_response)
    {
        this->action = action;

        std::string full_msg;
        
        full_msg.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");

        full_msg.append(this->request_body);

        std::string http_header("POST "+actionPOSTURLs[action]+" HTTP/1.1\r\n");
        if(action != AUTH)
        {
            http_header.append("SOAPAction: "+actionURLs[action]+"\r\n");
        }
        http_header.append(
                    "Accept: */*\r\n"
                    "Content-Type: text/xml; charset=utf-8\r\n"
                    "Cache-Control: no-cache\r\n"
                    "Proxy-Connection: Keep-Alive\r\n"
                    "Connection: Keep-Alive\r\n"
                    "User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 1.1.4322; .NET CLR 2.0.50727; Windows Live Messenger 8.1.0178)\r\n"
                    "Host: "+actionDomains[action]+"\r\n"
                    "Content-Length: " +toStr(full_msg.length()) +"\r\n\r\n"
                    );

        if ((this->sock = this->myNotificationServer()->externalCallbacks.connectToServer(actionDomains[action], 443, &this->connected, true)) == NULL)
        {
            this->myNotificationServer()->externalCallbacks.showError(this, "Could not connect to server");
            return;
        }
           this->myNotificationServer()->externalCallbacks.registerSocket(this->sock, 0, 1, true);

        if (this->connected)
            this->socketConnectionCompleted();

        std::ostringstream buf_;
        buf_ << http_header << full_msg;
        if(this->write(buf_) != buf_.str().size())
            return;

        this->myNotificationServer()->addSoapConnection(this);

    }

    void Soap::setMBI(std::string MBI)
    {
        this->mbi = MBI;
        for(unsigned int d=0; d< sitesToAuthList.size(); d++)
        {    
            // we just receive this MBI at connection time
            if(sitesToAuthList[d].url=="messengerclear.live.com")
                sitesToAuthList[d].URI=MBI;
        }
    }

    void Soap::fillURLs()
    {
        sitesToAuth sta;
    
        actionDomains[AUTH] = "login.live.com";
        actionDomains[GET_LISTS] = "by5.omega.contacts.msn.com";
        actionDomains[GET_ADDRESS_BOOK] = "by5.omega.contacts.msn.com";
        actionDomains[ADD_CONTACT_TO_LIST] = "by5.omega.contacts.msn.com";
        actionDomains[DEL_CONTACT_FROM_LIST] = "by5.omega.contacts.msn.com";
        actionDomains[DEL_CONTACT_FROM_ADDRESSBOOK] = "by5.omega.contacts.msn.com";
        actionDomains[ADD_CONTACT_TO_ADDRESSBOOK] = "by5.omega.contacts.msn.com";
        actionDomains[DISABLE_CONTACT_ON_ADDRESSBOOK] = "by5.omega.contacts.msn.com";
        actionDomains[ENABLE_CONTACT_ON_ADDRESSBOOK] = "by5.omega.contacts.msn.com";
        actionDomains[ADD_GROUP] = "by5.omega.contacts.msn.com";
        actionDomains[DEL_GROUP] = "by5.omega.contacts.msn.com";
        actionDomains[RENAME_GROUP] = "by5.omega.contacts.msn.com";
        actionDomains[BLOCK_CONTACT] = "";
        actionDomains[UNBLOCK_CONTACT] = "";
        actionDomains[ADD_CONTACT_TO_GROUP] = "by5.omega.contacts.msn.com";
        actionDomains[DEL_CONTACT_FROM_GROUP] ="by5.omega.contacts.msn.com";
        actionDomains[UPDATE_GROUP] = "";
        actionDomains[GENERATE_LOCKKEY] = "ows.messenger.msn.com";
        actionDomains[RETRIEVE_OIM_MAIL_DATA] = "rsi.hotmail.com";
        actionDomains[RETRIEVE_OIM] = "rsi.hotmail.com";
        actionDomains[DELETE_OIM] = "rsi.hotmail.com";
        actionDomains[SEND_OIM] = "ows.messenger.msn.com";
        actionDomains[CHANGE_DISPLAYNAME] = "by5.omega.contacts.msn.com";

        actionPOSTURLs[AUTH] = "/RST.srf";
        actionPOSTURLs[GET_LISTS] = "/abservice/SharingService.asmx";
        actionPOSTURLs[GET_ADDRESS_BOOK] = "/abservice/abservice.asmx";
        actionPOSTURLs[ADD_CONTACT_TO_LIST] = "/abservice/SharingService.asmx";
        actionPOSTURLs[DEL_CONTACT_FROM_LIST] = "/abservice/SharingService.asmx";
        actionPOSTURLs[DEL_CONTACT_FROM_ADDRESSBOOK] = "/abservice/abservice.asmx";
        actionPOSTURLs[ADD_CONTACT_TO_ADDRESSBOOK] = "/abservice/abservice.asmx";
        actionPOSTURLs[DISABLE_CONTACT_ON_ADDRESSBOOK] = "/abservice/abservice.asmx";
        actionPOSTURLs[ENABLE_CONTACT_ON_ADDRESSBOOK] = "/abservice/abservice.asmx";
        actionPOSTURLs[ADD_GROUP] = "/abservice/abservice.asmx";
        actionPOSTURLs[DEL_GROUP] = "/abservice/abservice.asmx";
        actionPOSTURLs[RENAME_GROUP] = "/abservice/abservice.asmx";
        actionPOSTURLs[BLOCK_CONTACT] = "";
        actionPOSTURLs[UNBLOCK_CONTACT] = "";
        actionPOSTURLs[ADD_CONTACT_TO_GROUP] = "/abservice/abservice.asmx";
        actionPOSTURLs[DEL_CONTACT_FROM_GROUP] ="/abservice/abservice.asmx";
        actionPOSTURLs[UPDATE_GROUP] = "";
        actionPOSTURLs[GENERATE_LOCKKEY] = "/OimWS/oim.asmx";
        actionPOSTURLs[RETRIEVE_OIM_MAIL_DATA] = "/rsi/rsi.asmx";
        actionPOSTURLs[RETRIEVE_OIM] = "/rsi/rsi.asmx";
        actionPOSTURLs[DELETE_OIM] = "/rsi/rsi.asmx";
        actionPOSTURLs[SEND_OIM] = "/OimWS/oim.asmx";
        actionPOSTURLs[CHANGE_DISPLAYNAME] = "/abservice/abservice.asmx";

        actionURLs[AUTH] = '\0';
        actionURLs[GET_LISTS] = "http://www.msn.com/webservices/AddressBook/FindMembership";
        actionURLs[GET_ADDRESS_BOOK] = "http://www.msn.com/webservices/AddressBook/ABFindAll";
        actionURLs[ADD_CONTACT_TO_LIST] = "http://www.msn.com/webservices/AddressBook/AddMember";
        actionURLs[DEL_CONTACT_FROM_LIST] = "http://www.msn.com/webservices/AddressBook/DeleteMember";
        actionURLs[DEL_CONTACT_FROM_ADDRESSBOOK] = "http://www.msn.com/webservices/AddressBook/ABContactDelete";
        actionURLs[ADD_CONTACT_TO_ADDRESSBOOK] = "http://www.msn.com/webservices/AddressBook/ABContactAdd";
        actionURLs[DISABLE_CONTACT_ON_ADDRESSBOOK] = "http://www.msn.com/webservices/AddressBook/ABContactUpdate";
        actionURLs[ENABLE_CONTACT_ON_ADDRESSBOOK] = "http://www.msn.com/webservices/AddressBook/ABContactUpdate";
        actionURLs[DEL_CONTACT_FROM_LIST] = "http://www.msn.com/webservices/AddressBook/DeleteMember";
        actionURLs[ADD_GROUP] = "http://www.msn.com/webservices/AddressBook/ABGroupAdd";
        actionURLs[DEL_GROUP] = "http://www.msn.com/webservices/AddressBook/ABGroupDelete";
        actionURLs[RENAME_GROUP] = "http://www.msn.com/webservices/AddressBook/ABGroupUpdate";
        actionURLs[BLOCK_CONTACT] = "";
        actionURLs[UNBLOCK_CONTACT] = "";
        actionURLs[ADD_CONTACT_TO_GROUP] = "http://www.msn.com/webservices/AddressBook/ABGroupContactAdd";
        actionURLs[DEL_CONTACT_FROM_GROUP] ="http://www.msn.com/webservices/AddressBook/ABGroupContactDelete";
        actionURLs[UPDATE_GROUP] = "";
        actionURLs[GENERATE_LOCKKEY] = "http://messenger.live.com/ws/2006/09/oim/Store2";
        actionURLs[RETRIEVE_OIM_MAIL_DATA] = "http://www.hotmail.msn.com/ws/2004/09/oim/rsi/GetMetadata";
        actionURLs[RETRIEVE_OIM] = "http://www.hotmail.msn.com/ws/2004/09/oim/rsi/GetMessage";
        actionURLs[DELETE_OIM] = "http://www.hotmail.msn.com/ws/2004/09/oim/rsi/DeleteMessages";
        actionURLs[SEND_OIM] = "http://messenger.live.com/ws/2006/09/oim/Store2";
        actionURLs[CHANGE_DISPLAYNAME] = "http://www.msn.com/webservices/AddressBook/ABContactUpdate";

        sta.url = "http://Passport.NET/tb";
        sitesToAuthList.push_back(sta);
        sta.url = "messengerclear.live.com";
        sta.URI = ""; // this is filled later.
        sitesToAuthList.push_back(sta);
        sta.url = "messenger.msn.com";
        sta.URI = "?id=507";
        sitesToAuthList.push_back(sta);
        sta.url = "contacts.msn.com";
        sta.URI = "MBI";
        sitesToAuthList.push_back(sta);
        sta.url = "messengersecure.live.com";
        sta.URI = "MBI_SSL";
        sitesToAuthList.push_back(sta);
        sta.url = "spaces.live.com";
        sta.URI = "MBI";
        sitesToAuthList.push_back(sta);
        sta.url = "storage.msn.com";
        sta.URI = "MBI";
        sitesToAuthList.push_back(sta);

    }

    void Soap::getTickets(std::string Passport, std::string password, std::string policy)
    {
        this->passport = Passport;
        this->password = password;
        this->policy = policy;
        XMLNode temp; //to general use
        XMLNode envelope = XMLNode::createXMLTopNode("Envelope");
        envelope.addAttribute("xmlns", "http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:wsse", "http://schemas.xmlsoap.org/ws/2003/06/secext");
        envelope.addAttribute("xmlns:saml", "urn:oasis:names:tc:SAML:1.0:assertion");
        envelope.addAttribute("xmlns:wsp", "http://schemas.xmlsoap.org/ws/2002/12/policy");
        envelope.addAttribute("xmlns:wsu", "http://docs.oasis-open.org/wss/2004/01/oasis-200401-wss-wssecurity-utility-1.0.xsd");
        envelope.addAttribute("xmlns:wsa", "http://schemas.xmlsoap.org/ws/2004/03/addressing");
        envelope.addAttribute("xmlns:wssc","http://schemas.xmlsoap.org/ws/2004/04/sc");
        envelope.addAttribute("xmlns:wst","http://schemas.xmlsoap.org/ws/2004/04/trust");
          XMLNode header = XMLNode::createXMLTopNode("Header");
            XMLNode authinfo = XMLNode::createXMLTopNode("ps:AuthInfo");
                authinfo.addAttribute("xmlns:ps","http://schemas.microsoft.com/Passport/SoapServices/PPCRL");
                authinfo.addAttribute("Id","PPAuthInfo");
                temp = XMLNode::createXMLTopNode("ps:HostingApp");
                temp.addText("{7108E71A-9926-4FCB-BCC9-9A9D3F32E423}");
                authinfo.addChild(temp);
                temp = XMLNode::createXMLTopNode("ps:BinaryVersion");
                temp.addText("4");
                authinfo.addChild(temp);
                temp = XMLNode::createXMLTopNode("ps:UIVersion");
                temp.addText("1");
                authinfo.addChild(temp);
                temp = XMLNode::createXMLTopNode("ps:Cookies");
                temp.addText("");
                authinfo.addChild(temp);
                temp = XMLNode::createXMLTopNode("ps:RequestParams");
                temp.addText("AQAAAAIAAABsYwQAAAAxMDMz");
                authinfo.addChild(temp);
               header.addChild(authinfo);

            XMLNode security = XMLNode::createXMLTopNode("wsse:Security");
               XMLNode username = XMLNode::createXMLTopNode("wsse:UsernameToken");
               username.addAttribute("Id","user");
                 temp = XMLNode::createXMLTopNode("wsse:Username");
                 temp.addText( Passport.c_str() );
                 username.addChild(temp);
                 temp = XMLNode::createXMLTopNode( "wsse:Password" );
                 temp.addText( password.c_str() );
                 username.addChild(temp);
               security.addChild(username);
            header.addChild(security);
              envelope.addChild( header );
          
          // BODY
          XMLNode body = XMLNode::createXMLTopNode( "Body" );
            XMLNode multipletokens = XMLNode::createXMLTopNode( "ps:RequestMultipleSecurityTokens" );
                multipletokens.addAttribute("xmlns:ps","http://schemas.microsoft.com/Passport/SoapServices/PPCRL");
                multipletokens.addAttribute("Id","RSTS");
            XMLNode securitytoken;
            XMLNode endpr;
            XMLNode address;

            // request tokens for each site
            for (unsigned int i=0; i<sitesToAuthList.size(); i++)
            {
                securitytoken = XMLNode::createXMLTopNode("wst:RequestSecurityToken");
                std::string RST = "RST";
                RST=RST+toStr(i);
                securitytoken.addAttribute("Id", RST.c_str() );
                temp = XMLNode::createXMLTopNode( "wst:RequestType" );
                temp.addText( "http://schemas.xmlsoap.org/ws/2004/04/security/trust/Issue" );
                securitytoken.addChild(temp);

                temp = XMLNode::createXMLTopNode( "wsp:AppliesTo" );
                    endpr = XMLNode::createXMLTopNode("wsa:EndpointReference");
                        address = XMLNode::createXMLTopNode("wsa:Address");
                        address.addText( sitesToAuthList[i].url.c_str() );
                    endpr.addChild(address);
                temp.addChild(endpr);
                securitytoken.addChild(temp);
                if(!sitesToAuthList[i].URI.empty())
                {
                    XMLNode policyref = XMLNode::createXMLTopNode("wsse:PolicyReference");
                    policyref.addAttribute("URI", sitesToAuthList[i].URI.c_str());
                    policyref.addText( "" );
                    securitytoken.addChild(policyref);
                }
                multipletokens.addChild(securitytoken);
            }

        body.addChild(multipletokens);
      envelope.addChild( body );

      std::string xml_response;
      char *xml_request = envelope.createXMLString(false);
      std::string temp2 = xml_request;
      this->request_body = temp2;

      requestSoapAction(AUTH, xml_request, xml_response);

      free(xml_request);
      envelope.deleteNodeContent();
    }

    void Soap::parseGetTicketsResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString( response.c_str() );
        if(http_response_code == "301")
        {
            Soap *soapConnection = manageSoapRedirect(response1, AUTH);
            soapConnection->getTickets(this->passport, this->password, this->policy);
            return;
        }

          // get the header information from the DOM
          XMLNode tokens = response1.getChildNode("S:Envelope").getChildNode("S:Body").getChildNode("wst:RequestSecurityTokenResponseCollection");
          const char *reason = response1.getChildNode("S:Envelope").getChildNode("S:Fault").getChildNode("faultcode").getText();
          if(reason)
          {
            std::string reason1(reason);
            if(reason1 == "wsse:FailedAuthentication")
            {
                this->myNotificationServer()->externalCallbacks.showError(this, "Wrong Password");
                this->myNotificationServer()->removeSoapConnection(this);
                this->myNotificationServer()->disconnect();
                return;
            }
            if(reason1 == "psf:Redirect")
            {
                const char *newurl = response1.getChildNode("S:Envelope").getChildNode("S:Fault").getChildNode("psf:redirectUrl").getText();
                Soap *soapConnection = new Soap(notificationServer);

                std::string newurl1(newurl);
                std::vector<std::string> a = splitString(newurl1, "/");
                std::string newdomain = splitString(a[1], "/")[0];
                soapConnection->actionDomains[AUTH] = newdomain;
                std::vector<std::string> postpath = splitString(newurl1, newdomain);
                soapConnection->actionPOSTURLs[AUTH] = postpath[1];
                soapConnection->setMBI(mbi);

                soapConnection->getTickets(passport,password,policy);
                return;
            }
          }
          int nItems = tokens.nChildNode("wst:RequestSecurityTokenResponse");

          // fill sitesToAuthList the strucutre with tokens and binary secrets
          for(int site=0; site< nItems; site++)
          {
              XMLNode a = tokens.getChildNode("wst:RequestSecurityTokenResponse",site);

              const char *reason = a.getChildNode("S:Fault").getChildNode("faultcode").getText();
              if(reason)
              {
                std::string reason1(reason);
                if(reason1 == "wsse:FailedAuthentication")
                {
                  const char *reasonString = a.getChildNode("S:Fault").getChildNode("faultstring").getText();
                  std::string reasonString1 = (reasonString) ? reasonString : "Authentication Failed";
                  this->myNotificationServer()->externalCallbacks.showError(this, reasonString1);
                  this->myNotificationServer()->removeSoapConnection(this);
                  this->myNotificationServer()->disconnect();
                  return;
                }
              }
              const char *token1 = a.getChildNode("wst:RequestedProofToken").getChildNode("wst:BinarySecret").getText();
              if(token1)
              {
                  std::string c(token1);
                  sitesToAuthList[site].BinarySecret = c;
              }
              const char *token2 = a.getChildNode("wst:RequestedSecurityToken").getChildNode("wsse:BinarySecurityToken").getText();
              if(token2)
              {
                std::string b(token2);
                sitesToAuthList[site].BinarySecurityToken = b;
              }
          }
          this->myNotificationServer()->gotTickets(*this, sitesToAuthList);
    }

    void Soap::enableContactOnAddressBook(std::string contactId, std::string passport, std::string myDisplayName)
    {
        this->contactId = contactId;
        this->tempPassport = passport;
        this->myDisplayName = myDisplayName;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("ContactSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABContactUpdate = XMLNode::createXMLTopNode( "ABContactUpdate" );
            ABContactUpdate.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABContactUpdate.addChild(abId);
                XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
            Contact.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode contactId1 = XMLNode::createXMLTopNode( "contactId" );
              contactId1.addText(contactId.c_str());
                  XMLNode contactInfo = XMLNode::createXMLTopNode( "contactInfo" );
                    XMLNode displayName = XMLNode::createXMLTopNode( "displayName" );
                displayName.addText(passport.c_str());
                    XMLNode isMessengerUser = XMLNode::createXMLTopNode( "isMessengerUser" );
                      isMessengerUser.addText("true");
                    XMLNode MessengerMemberInfo = XMLNode::createXMLTopNode( "MessengerMemberInfo" );
                      XMLNode DisplayName = XMLNode::createXMLTopNode( "DisplayName" );
                  DisplayName.addText(myDisplayName.c_str());
                MessengerMemberInfo.addChild(DisplayName);
              contactInfo.addChild(displayName);
              contactInfo.addChild(isMessengerUser);
              contactInfo.addChild(MessengerMemberInfo);
                  XMLNode propertiesChanged = XMLNode::createXMLTopNode( "propertiesChanged" );
                      propertiesChanged.addText("DisplayName IsMessengerUser MessengerMemberInfo");
                Contact.addChild(contactId1);
                Contact.addChild(contactInfo);
                Contact.addChild(propertiesChanged);
             contacts.addChild(Contact);
            ABContactUpdate.addChild(contacts);
           body.addChild(ABContactUpdate);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(ENABLE_CONTACT_ON_ADDRESSBOOK, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseEnableContactOnAddressBookResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, ENABLE_CONTACT_ON_ADDRESSBOOK);
            soapConnection->enableContactOnAddressBook(this->contactId, this->tempPassport, this->myDisplayName);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotEnableContactOnAddressBookConfirmation(*this, true, newVersion, this->contactId, this->tempPassport);
        }
        else
        {
            this->myNotificationServer()->gotEnableContactOnAddressBookConfirmation(*this, false, "", this->contactId, this->tempPassport);
        }
        response1.deleteNodeContent();
    }

    void Soap::delContactFromAddressBook(std::string contactId, std::string passport)
    {
        this->contactId = contactId;
        this->tempPassport = passport;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("Timer");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABContactDelete = XMLNode::createXMLTopNode( "ABContactDelete" );
            ABContactDelete.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABContactDelete.addChild(abId);
                XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
              XMLNode contactId1 = XMLNode::createXMLTopNode( "contactId" );
              contactId1.addText(contactId.c_str());
                Contact.addChild(contactId1);
             contacts.addChild(Contact);
            ABContactDelete.addChild(contacts);
           body.addChild(ABContactDelete);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(DEL_CONTACT_FROM_ADDRESSBOOK, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    
    }

    void Soap::parseDelContactFromAddressBookResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, DEL_CONTACT_FROM_ADDRESSBOOK);
            soapConnection->delContactFromAddressBook(this->contactId, this->tempPassport);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotDelContactFromAddressBookConfirmation(*this, true, newVersion, this->contactId, this->tempPassport);
        }
        else
        {
            this->myNotificationServer()->gotDelContactFromAddressBookConfirmation(*this, false, "", this->contactId, this->tempPassport);
        }
        response1.deleteNodeContent();

    }

    void Soap::disableContactFromAddressBook(std::string contactId, std::string passport)
    {
        this->contactId = contactId;
        this->tempPassport = passport;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("Timer");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABContactUpdate = XMLNode::createXMLTopNode( "ABContactUpdate" );
            ABContactUpdate.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABContactUpdate.addChild(abId);
                XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
            Contact.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode contactId1 = XMLNode::createXMLTopNode( "contactId" );
              contactId1.addText(contactId.c_str());
                  XMLNode contactInfo = XMLNode::createXMLTopNode( "contactInfo" );
                    XMLNode displayName = XMLNode::createXMLTopNode( "displayName" );
                    XMLNode isMessengerUser = XMLNode::createXMLTopNode( "isMessengerUser" );
                      isMessengerUser.addText("false");
              contactInfo.addChild(displayName);
              contactInfo.addChild(isMessengerUser);
                  XMLNode propertiesChanged = XMLNode::createXMLTopNode( "propertiesChanged" );
                      propertiesChanged.addText("DisplayName IsMessengerUser");
                Contact.addChild(contactId1);
                Contact.addChild(contactInfo);
                Contact.addChild(propertiesChanged);
             contacts.addChild(Contact);
            ABContactUpdate.addChild(contacts);
           body.addChild(ABContactUpdate);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(DISABLE_CONTACT_ON_ADDRESSBOOK, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseDisableContactFromAddressBookResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, DISABLE_CONTACT_ON_ADDRESSBOOK);
            soapConnection->disableContactFromAddressBook(this->contactId, this->tempPassport);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotDisableContactOnAddressBookConfirmation(*this, true, newVersion, this->contactId, this->tempPassport);
        }
        else
        {
            this->myNotificationServer()->gotDisableContactOnAddressBookConfirmation(*this, false, "", this->contactId, this->tempPassport);
        }
        response1.deleteNodeContent();
    }

    void Soap::addContactToAddressBook(std::string passport, std::string displayName)
    {
        this->tempPassport = passport;
        this->tempDisplayName = displayName;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("ContactSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABContactAdd = XMLNode::createXMLTopNode( "ABContactAdd" );
            ABContactAdd.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABContactAdd.addChild(abId);
                XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
            Contact.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
                  XMLNode contactInfo = XMLNode::createXMLTopNode( "contactInfo" );
                    XMLNode contactType = XMLNode::createXMLTopNode( "contactType" );
                      contactType.addText("Regular");
                    XMLNode passportName = XMLNode::createXMLTopNode( "passportName" );
                      passportName.addText(passport.c_str());
                    XMLNode isMessengerUser = XMLNode::createXMLTopNode( "isMessengerUser" );
                      isMessengerUser.addText("true");
                    XMLNode MessengerMemberInfo = XMLNode::createXMLTopNode( "MessengerMemberInfo" );
                  XMLNode DisplayName = XMLNode::createXMLTopNode( "DisplayName" );
                    DisplayName.addText(displayName.c_str());
                MessengerMemberInfo.addChild(DisplayName);
              contactInfo.addChild(contactType);
              contactInfo.addChild(passportName);
              contactInfo.addChild(isMessengerUser);
              contactInfo.addChild(MessengerMemberInfo);
                Contact.addChild(contactInfo);
             contacts.addChild(Contact);
            ABContactAdd.addChild(contacts);
             XMLNode options = XMLNode::createXMLTopNode( "options" );
               XMLNode EnableAllowListManagement = XMLNode::createXMLTopNode( "EnableAllowListManagement" );
               EnableAllowListManagement.addText("true");
             options.addChild(EnableAllowListManagement);
            ABContactAdd.addChild(options);
           body.addChild(ABContactAdd);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(ADD_CONTACT_TO_ADDRESSBOOK, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseAddContactToAddressBookResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, ADD_CONTACT_TO_ADDRESSBOOK);
            soapConnection->addContactToAddressBook(this->tempPassport, this->tempDisplayName);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            const char *guid = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("ABContactAddResponse").getChildNode("ABContactAddResult").getChildNode("guid").getText();
            if(guid)
            {
                std::string newVersion(ver);
                std::string newGuid(guid);
                this->myNotificationServer()->gotAddContactToAddressBookConfirmation(*this, true, newVersion, this->tempPassport, this->tempDisplayName, newGuid);
            }
        }
        else
        {
            this->myNotificationServer()->gotAddContactToAddressBookConfirmation(*this, false, "", this->tempPassport, this->tempDisplayName, "");
        }
        response1.deleteNodeContent();
    }

    void Soap::addContactToGroup(std::string groupId, std::string contactId)
    {
        this->groupId = groupId;
        this->contactId = contactId;
        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("GroupSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABGroupContactAdd = XMLNode::createXMLTopNode( "ABGroupContactAdd" );
            ABGroupContactAdd.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABGroupContactAdd.addChild(abId);
              XMLNode groupFilter = XMLNode::createXMLTopNode( "groupFilter" );
                XMLNode groupIds = XMLNode::createXMLTopNode( "groupIds" );
                  XMLNode guid = XMLNode::createXMLTopNode( "guid" );
                    guid.addText(groupId.c_str());
            groupIds.addChild(guid);
              groupFilter.addChild(groupIds);
            ABGroupContactAdd.addChild(groupFilter);
                XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
                  XMLNode contactId1 = XMLNode::createXMLTopNode( "contactId" );
                    contactId1.addText(contactId.c_str());
            Contact.addChild(contactId1);
             contacts.addChild(Contact);
            ABGroupContactAdd.addChild(contacts);
           body.addChild(ABGroupContactAdd);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(ADD_CONTACT_TO_GROUP, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseAddContactToGroupResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, ADD_CONTACT_TO_GROUP);
            soapConnection->addContactToGroup(this->groupId, this->contactId);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotAddContactToGroupConfirmation(*this, true, newVersion, this->groupId, this->contactId);
        }
        else
        {
            this->myNotificationServer()->gotAddContactToGroupConfirmation(*this, false, "", this->groupId, this->contactId);
        }
        response1.deleteNodeContent();
    }

    void Soap::addGroup(std::string groupName)
    {
        this->groupName = groupName;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("GroupSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABGroupAdd = XMLNode::createXMLTopNode( "ABGroupAdd" );
            ABGroupAdd.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABGroupAdd.addChild(abId);
              XMLNode groupAddOptions = XMLNode::createXMLTopNode( "groupAddOptions" );
                XMLNode fRenameOnMsgrConflict = XMLNode::createXMLTopNode( "fRenameOnMsgrConflict" );
                    fRenameOnMsgrConflict.addText("false");
            groupAddOptions.addChild(fRenameOnMsgrConflict);
            ABGroupAdd.addChild(groupAddOptions);
                XMLNode groupInfo = XMLNode::createXMLTopNode( "groupInfo" );
                      XMLNode GroupInfo = XMLNode::createXMLTopNode( "GroupInfo" );
                    XMLNode name = XMLNode::createXMLTopNode( "name" );
                name.addText(groupName.c_str());
              GroupInfo.addChild(name);
                    XMLNode groupType = XMLNode::createXMLTopNode( "groupType" );
                groupType.addText("C8529CE2-6EAD-434d-881F-341E17DB3FF8");
              GroupInfo.addChild(groupType);
                    XMLNode fMessenger = XMLNode::createXMLTopNode( "fMessenger" );
                fMessenger.addText("false");
              GroupInfo.addChild(fMessenger);
                    XMLNode annotations = XMLNode::createXMLTopNode( "annotations" );
                          XMLNode Annotation = XMLNode::createXMLTopNode( "Annotation" );
                        XMLNode Name = XMLNode::createXMLTopNode( "Name" );
                      Name.addText("MSN.IM.Display");
                        XMLNode Value = XMLNode::createXMLTopNode( "Value" );
                      Value.addText("1");
                  Annotation.addChild(Name);
                  Annotation.addChild(Value);
                annotations.addChild(Annotation);  
              GroupInfo.addChild(annotations);
            groupInfo.addChild(GroupInfo);
             ABGroupAdd.addChild(groupInfo);
           body.addChild(ABGroupAdd);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(ADD_GROUP, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseAddGroupResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, ADD_GROUP);
            soapConnection->addGroup(this->groupName);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            const char *guid = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("ABGroupAddResponse").getChildNode("ABGroupAddResult").getChildNode("guid").getText();
            if(guid)
            {
                std::string newVersion(ver);
                std::string newGuid(guid);
                this->myNotificationServer()->gotAddGroupConfirmation(*this, true, newVersion, this->groupName, newGuid);
            }
        }
        else
        {
            this->myNotificationServer()->gotAddGroupConfirmation(*this, false, "", this->groupName, "");
        }
        response1.deleteNodeContent();
    }

    void Soap::delGroup(std::string groupId)
    {
        this->groupId = groupId;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("GroupSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABGroupDelete = XMLNode::createXMLTopNode( "ABGroupDelete" );
            ABGroupDelete.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABGroupDelete.addChild(abId);
              XMLNode groupFilter = XMLNode::createXMLTopNode( "groupFilter" );
                XMLNode groupIds = XMLNode::createXMLTopNode( "groupIds" );
                      XMLNode guid = XMLNode::createXMLTopNode( "guid" );
                guid.addText(groupId.c_str());
            groupIds.addChild(guid);
              groupFilter.addChild(groupIds);
            ABGroupDelete.addChild(groupFilter);
           body.addChild(ABGroupDelete);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(DEL_GROUP, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseDelGroupResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, DEL_GROUP);
            soapConnection->delGroup(this->groupId);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotDelGroupConfirmation(*this, true, newVersion, this->groupId);
        }
        else
        {
            this->myNotificationServer()->gotDelGroupConfirmation(*this, false, "", this->groupId);
        }

        response1.deleteNodeContent();
    }

    void Soap::renameGroup(std::string groupId, std::string newGroupName)
    {
        this->groupId = groupId;
        this->groupName = newGroupName;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("GroupSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABGroupUpdate = XMLNode::createXMLTopNode( "ABGroupUpdate" );
            ABGroupUpdate.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABGroupUpdate.addChild(abId);
              XMLNode groups = XMLNode::createXMLTopNode( "groups" );
                XMLNode Group = XMLNode::createXMLTopNode( "Group" );
                      XMLNode groupId1 = XMLNode::createXMLTopNode( "groupId" );
              groupId1.addText(groupId.c_str());
                      XMLNode groupInfo = XMLNode::createXMLTopNode( "groupInfo" );
                XMLNode name = XMLNode::createXMLTopNode( "name" );
                name.addText(newGroupName.c_str());
              groupInfo.addChild(name);
                      XMLNode propertiesChanged = XMLNode::createXMLTopNode( "propertiesChanged" );
              propertiesChanged.addText("GroupName");
            Group.addChild(groupId1);
            Group.addChild(groupInfo);
            Group.addChild(propertiesChanged);
              groups.addChild(Group);
            ABGroupUpdate.addChild(groups);
           body.addChild(ABGroupUpdate);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(RENAME_GROUP, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseRenameGroupResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, RENAME_GROUP);
            soapConnection->renameGroup(this->groupId, this->groupName);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotRenameGroupConfirmation(*this, true, newVersion, this->groupName, this->groupId);
        }
        else
        {
            this->myNotificationServer()->gotRenameGroupConfirmation(*this, false, "", this->groupName, this->groupId);
        }
        response1.deleteNodeContent();
    }


    void Soap::delContactFromGroup(std::string groupId, std::string contactId)
    {
        this->groupId = groupId;
        this->contactId = contactId;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("GroupSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode ABGroupContactDelete = XMLNode::createXMLTopNode( "ABGroupContactDelete" );
            ABGroupContactDelete.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                abId.addText( "00000000-0000-0000-0000-000000000000" );
            ABGroupContactDelete.addChild(abId);
                XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
                  XMLNode contactId1 = XMLNode::createXMLTopNode( "contactId" );
                    contactId1.addText(contactId.c_str());
            Contact.addChild(contactId1);
             contacts.addChild(Contact);
            ABGroupContactDelete.addChild(contacts);
              XMLNode groupFilter = XMLNode::createXMLTopNode( "groupFilter" );
                XMLNode groupIds = XMLNode::createXMLTopNode( "groupIds" );
                  XMLNode guid = XMLNode::createXMLTopNode( "guid" );
                    guid.addText(groupId.c_str());
            groupIds.addChild(guid);
              groupFilter.addChild(groupIds);
            ABGroupContactDelete.addChild(groupFilter);
           body.addChild(ABGroupContactDelete);
         envelope.addChild(body);

         std::string xml_response;
         char *xml_request = envelope.createXMLString(false);
         std::string temp2 = xml_request;
         this->request_body = temp2;

         requestSoapAction(DEL_CONTACT_FROM_GROUP, xml_request, xml_response);

         free(xml_request);
         envelope.deleteNodeContent();
    }

    void Soap::parseDelContactFromGroupResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, DEL_CONTACT_FROM_GROUP);
            soapConnection->delContactFromGroup(this->groupId, this->contactId);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotDelContactFromGroupConfirmation(*this, true, newVersion, this->groupId, this->contactId);
        }
        else
        {
            this->myNotificationServer()->gotDelContactFromGroupConfirmation(*this, false, "", this->groupId, this->contactId);
        }
        response1.deleteNodeContent();
    }

    void Soap::addContactToList(MSN::Passport passport, MSN::ContactList list)
    {
        this->tempPassport = passport;
        this->tempList = list;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("ContactSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode AddMember = XMLNode::createXMLTopNode( "AddMember" );
            AddMember.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode serviceHandle = XMLNode::createXMLTopNode( "serviceHandle" );
               XMLNode Id = XMLNode::createXMLTopNode( "Id" );
                  Id.addText( "0" );
               XMLNode Type = XMLNode::createXMLTopNode( "Type" );
                  Type.addText( "Messenger" );
               XMLNode ForeignId = XMLNode::createXMLTopNode( "ForeignId" );
                  ForeignId.addText( "" );
               serviceHandle.addChild(Id);
               serviceHandle.addChild(Type);
               serviceHandle.addChild(ForeignId);
            AddMember.addChild(serviceHandle);
               XMLNode memberships = XMLNode::createXMLTopNode( "memberships" );
                 XMLNode Membership = XMLNode::createXMLTopNode( "Membership" );
                   XMLNode MemberRole = XMLNode::createXMLTopNode( "MemberRole" );
               switch(list)
               {
                   case LST_AL:
                       MemberRole.addText("Allow");
                    break;
                   case LST_BL:
                       MemberRole.addText("Block");
                    break;
                   case LST_RL:
                       MemberRole.addText("Reverse");
                    break;
                   default:
                    return; // TODO - raise an error
               }
               XMLNode Members = XMLNode::createXMLTopNode( "Members" );
                 XMLNode Member = XMLNode::createXMLTopNode( "Member" );
                 Member.addAttribute("xsi:type","PassportMember");
                 Member.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
                   XMLNode Type2 = XMLNode::createXMLTopNode("Type");
                   Type2.addText("Passport");
                   XMLNode State = XMLNode::createXMLTopNode("State");
                   State.addText("Accepted");
                   XMLNode PassportName = XMLNode::createXMLTopNode("PassportName");
                   PassportName.addText(passport.c_str());
                 Member.addChild(Type2);
                 Member.addChild(State);
                 Member.addChild(PassportName);
               Members.addChild(Member);
             Membership.addChild(MemberRole);
             Membership.addChild(Members);
               memberships.addChild(Membership);
               AddMember.addChild(memberships);
             body.addChild(AddMember);
           envelope.addChild(body);

           std::string xml_response;
           char *xml_request = envelope.createXMLString(false);
           std::string temp2 = xml_request;
           this->request_body = temp2;

           requestSoapAction(ADD_CONTACT_TO_LIST, xml_request, xml_response);

           free(xml_request);
           envelope.deleteNodeContent();
    }

    void Soap::parseAddContactToListResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301")
        {
            Soap *soapConnection = manageSoapRedirect(response1, ADD_CONTACT_TO_LIST);
            soapConnection->addContactToList(this->tempPassport, this->tempList);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotAddContactToListConfirmation(*this, true, newVersion, this->tempPassport, this->tempList);
        }
        else
        {
            this->myNotificationServer()->gotAddContactToListConfirmation(*this, false, "", this->tempPassport, this->tempList);
        }
        response1.deleteNodeContent();
    }

    void Soap::removeContactFromList(MSN::Passport passport, MSN::ContactList list)
    {
        this->tempPassport = passport;
        this->tempList = list;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false") ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText ("ContactSave");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
            XMLNode DeleteMember = XMLNode::createXMLTopNode( "DeleteMember" );
            DeleteMember.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode serviceHandle = XMLNode::createXMLTopNode( "serviceHandle" );
               XMLNode Id = XMLNode::createXMLTopNode( "Id" );
                  Id.addText( "0" );
               XMLNode Type = XMLNode::createXMLTopNode( "Type" );
                  Type.addText( "Messenger" );
               XMLNode ForeignId = XMLNode::createXMLTopNode( "ForeignId" );
                  Type.addText( "" );
               serviceHandle.addChild(Id);
               serviceHandle.addChild(Type);
               serviceHandle.addChild(ForeignId);
            DeleteMember.addChild(serviceHandle);
               XMLNode memberships = XMLNode::createXMLTopNode( "memberships" );
                 XMLNode Membership = XMLNode::createXMLTopNode( "Membership" );
                   XMLNode MemberRole = XMLNode::createXMLTopNode( "MemberRole" );
               switch(list)
               {
                   case LST_AL:
                       MemberRole.addText("Allow");
                    break;
                   case LST_BL:
                       MemberRole.addText("Block");
                    break;
                   case LST_PL:
                       MemberRole.addText("Pending");
                    break;
                   default:
                    return; // TODO - raise an error
               }
               XMLNode Members = XMLNode::createXMLTopNode( "Members" );
                 XMLNode Member = XMLNode::createXMLTopNode( "Member" );
                 Member.addAttribute("xsi:type","PassportMember");
                 Member.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
                   XMLNode Type2 = XMLNode::createXMLTopNode("Type");
                   Type2.addText("Passport");
                   XMLNode State = XMLNode::createXMLTopNode("State");
                   State.addText("Accepted");
                   XMLNode PassportName = XMLNode::createXMLTopNode("PassportName");
                   PassportName.addText(passport.c_str());
                 Member.addChild(Type2);
                 Member.addChild(State);
                 Member.addChild(PassportName);
               Members.addChild(Member);
             Membership.addChild(MemberRole);
             Membership.addChild(Members);
               memberships.addChild(Membership);
               DeleteMember.addChild(memberships);
             body.addChild(DeleteMember);
           envelope.addChild(body);

           std::string xml_response;
           char *xml_request = envelope.createXMLString(false);
           std::string temp2 = xml_request;
           this->request_body = temp2;

           requestSoapAction(DEL_CONTACT_FROM_LIST, xml_request, xml_response);

           free(xml_request);
           envelope.deleteNodeContent();

    }

    void Soap::parseRemoveContactFromListResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301")
        {
            Soap *soapConnection = manageSoapRedirect(response1, DEL_CONTACT_FROM_LIST);
            soapConnection->removeContactFromList(this->tempPassport, this->tempList);
            return;
        }

        XMLNode version = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("Version");
        const char *ver = version.getText();
        if(ver)
        {
            std::string newVersion(ver);
            this->myNotificationServer()->gotDelContactFromListConfirmation(*this, true, newVersion, this->tempPassport, this->tempList);
        }
        else
        {
            this->myNotificationServer()->gotDelContactFromListConfirmation(*this, false, "", this->tempPassport, this->tempList);
        }
        response1.deleteNodeContent();
    }

    void Soap::getLists(ListSyncInfo *data)
    {
        // info is used to to fill the lists with memebers
        this->listInfo = data;        

        XMLNode envelope = XMLNode::createXMLTopNode( "soap:Envelope" );
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
          XMLNode header = XMLNode::createXMLTopNode( "soap:Header" );
          header.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
            XMLNode abapphdr = XMLNode::createXMLTopNode( "ABApplicationHeader" );
            abapphdr.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
            XMLNode appid = XMLNode::createXMLTopNode( "ApplicationId" );
              appid.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              appid.addText( "996CDE1E-AA53-4477-B943-2BE802EA6166" );
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode( "IsMigration" );
              ismigration.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              ismigration.addText( "false" ) ;
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode( "PartnerScenario" );
              scenario.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              scenario.addText ("Initial" );
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode( "ABAuthHeader" );
            ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode( "ManagedGroupRequest" );
              ManagedGroupRequest.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              ManagedGroupRequest.addText( "false" );
              XMLNode TicketToken = XMLNode::createXMLTopNode( "TicketToken" );
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
          body.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
            XMLNode FindMembership = XMLNode::createXMLTopNode( "FindMembership" );
            FindMembership.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
              XMLNode serviceFilter = XMLNode::createXMLTopNode( "serviceFilter" );
                XMLNode Types = XMLNode::createXMLTopNode( "Types" );
                  XMLNode ServiceType = XMLNode::createXMLTopNode( "ServiceType" );
                  ServiceType.addText( "Messenger" );
                  Types.addChild(ServiceType);
                  ServiceType = XMLNode::createXMLTopNode( "ServiceType" );
                  ServiceType.addText( "Invitation" );
                  Types.addChild(ServiceType);
                  ServiceType = XMLNode::createXMLTopNode( "ServiceType" );
                  ServiceType.addText( "SocialNetwork" );
                  Types.addChild(ServiceType);
                  ServiceType = XMLNode::createXMLTopNode( "ServiceType" );
                  ServiceType.addText( "Space" );
                  Types.addChild(ServiceType);
                  ServiceType = XMLNode::createXMLTopNode( "ServiceType" );
                  ServiceType.addText( "Profile" );
                  Types.addChild(ServiceType);
            serviceFilter.addChild(Types);
            if(data->lastChange != "0")
            {
                   XMLNode View = XMLNode::createXMLTopNode( "View" );
               View.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
               View.addText("Full");
                   XMLNode deltasOnly = XMLNode::createXMLTopNode( "deltasOnly" );
               deltasOnly.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
               deltasOnly.addText("true");
                   XMLNode lastChange = XMLNode::createXMLTopNode( "lastChange" );
               lastChange.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
               lastChange.addText(data->lastChange.c_str());
               FindMembership.addChild(View);
               FindMembership.addChild(deltasOnly);
               FindMembership.addChild(lastChange);
            }
              FindMembership.addChild(serviceFilter);
                  body.addChild(FindMembership);
                envelope.addChild(body);

          std::string xml_response;
          char *xml_request = envelope.createXMLString(false);
          std::string temp2 = xml_request;
          this->request_body = temp2;

          requestSoapAction(GET_LISTS, xml_request, xml_response);

          free(xml_request);
          envelope.deleteNodeContent();
    }

    void Soap::parseGetListsResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());
        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, GET_LISTS);
            soapConnection->getLists(this->listInfo);
            return;
        }

        XMLNode Services = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("FindMembershipResponse").getChildNode("FindMembershipResult").getChildNode("Services");

        int nServices = Services.nChildNode("Service");
        for(int d=0;d<nServices;d++)
        {
            XMLNode Service = Services.getChildNode("Service", d);

            XMLNode Memberships = Service.getChildNode("Memberships");
            int nItems = Memberships.nChildNode("Membership");

            for(int i=0;i<nItems;i++)
            {
                XMLNode Membership = Memberships.getChildNode("Membership", i);
                std::string MemberRole = Membership.getChildNode("MemberRole").getText();
                XMLNode Members = Membership.getChildNode("Members");

                int nItems2 = Members.nChildNode("Member");
                for(int j=0; j<nItems2;j++)
                {
                    XMLNode Member = Members.getChildNode("Member", j);
                    // Email type is another network than WLM. Not supported yet
                    if(Member.nChildNode("Type") && 
                       Member.getChildNode("Type").getText() == "Email")
                            continue;

                    // TODO- verify if xsi:type == "PassportMember" instead
                    // of presence of PassportName
                    if(!Member.nChildNode("PassportName"))
                            continue;
                    std::string a = Member.getChildNode("PassportName").getText();
                    transform (a.begin(),a.end(), a.begin(), tolower); 

                    // we cant add this contact or we will receive a 205 error
                    // see http://trac.adiumx.com/ticket/11126
                    if(a == "messenger@microsoft.com")
                        continue;

                    if(!listInfo->contactList[a])
                        listInfo->contactList[a]=new Buddy(a);

                    if(MemberRole == "Allow")
                    {
                        listInfo->contactList[a]->lists |= MSN::LST_AL;
                    }
                    else if(MemberRole == "Block")
                    {
                        listInfo->contactList[a]->lists |= MSN::LST_BL;
                    }
                    else if(MemberRole == "Reverse")
                    {
                        listInfo->contactList[a]->lists |= MSN::LST_RL;
                    }
                    else if(MemberRole == "Pending")
                    {
                        listInfo->contactList[a]->lists |= MSN::LST_PL;
                    }

                }
            }
        }
        listInfo->progress |= ListSyncInfo::LST_RL | ListSyncInfo::LST_AL | ListSyncInfo::LST_BL | ListSyncInfo::LST_PL;
        response1.deleteNodeContent();
        this->myNotificationServer()->gotLists(*this);
    }

    void Soap::getAddressBook(ListSyncInfo *info)
    {
        this->listInfo = info;
        // info is used to to fill the lists with memebers
        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode abapphdr = XMLNode::createXMLTopNode("ABApplicationHeader");
            abapphdr.addAttribute("xmlns", "http://www.msn.com/webservices/AddressBook");
              XMLNode appid = XMLNode::createXMLTopNode("ApplicationId");
              appid.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
              abapphdr.addChild(appid);
              XMLNode ismigration = XMLNode::createXMLTopNode("IsMigration");
              ismigration.addText("false");
              abapphdr.addChild(ismigration);
              XMLNode scenario = XMLNode::createXMLTopNode("PartnerScenario");
              scenario.addText("Initial");
              abapphdr.addChild(scenario);
              header.addChild(abapphdr);
            XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
            ABAuthHeader.addAttribute("xmlns", "http://www.msn.com/webservices/AddressBook");
              XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode("ManagedGroupRequest");
              ManagedGroupRequest.addText("false");
              XMLNode TicketToken = XMLNode::createXMLTopNode("TicketToken");
              // TODO - change this - maybe one day the position can be changed
              TicketToken.addText( sitesToAuthList[3].BinarySecurityToken.c_str() );
              ABAuthHeader.addChild(ManagedGroupRequest);
              ABAuthHeader.addChild(TicketToken);
              header.addChild(ABAuthHeader);
            envelope.addChild(header);
           
          XMLNode body = XMLNode::createXMLTopNode("soap:Body");
            XMLNode ABFindAll = XMLNode::createXMLTopNode("FindMembership");
            ABFindAll.addAttribute("xmlns", "http://www.msn.com/webservices/AddressBook");
                  XMLNode abId = XMLNode::createXMLTopNode("abId");
                  abId.addText("00000000-0000-0000-0000-000000000000");
              ABFindAll.addChild(abId);
                  XMLNode abView = XMLNode::createXMLTopNode("abView");
                  abView.addText("Full");
              ABFindAll.addChild(abView);
                  XMLNode deltasOnly = XMLNode::createXMLTopNode("deltasOnly");
              if(info->lastChange != "0")
              {
                  // receive all the list
                deltasOnly.addText("true");
              }
              else
              {
                  // receive only the changes since lastChange
                      deltasOnly.addText("false");
              }
              ABFindAll.addChild(deltasOnly);
                  XMLNode lastChange_ = XMLNode::createXMLTopNode("lastChange");
              if(info->lastChange == "0")
              {
                      lastChange_.addText("0001-01-01T00:00:00.0000000-08:00");
              }
              else
              {
                      lastChange_.addText( info->lastChange.c_str() );
              }
              ABFindAll.addChild(lastChange_);
                  body.addChild(ABFindAll);
                envelope.addChild(body);  

          std::string xml_response;
          char *xml_request = envelope.createXMLString(false);
          std::string temp2 = xml_request;
          this->request_body = temp2;

          requestSoapAction(GET_ADDRESS_BOOK, xml_request, xml_response);

          free(xml_request);
          envelope.deleteNodeContent();
    }

    void Soap::parseGetAddressBookResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());
        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, GET_ADDRESS_BOOK);
            soapConnection->getAddressBook(this->listInfo);
            return;
        }

        XMLNode groups = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("ABFindAllResponse").getChildNode("ABFindAllResult").getChildNode("groups");
        int nItems = groups.nChildNode("Group");

        for(int i=0;i<nItems;i++)
        {
            XMLNode thisGroup = groups.getChildNode("Group", i);
            std::string groupId = thisGroup.getChildNode("groupId").getText();
            std::string groupName = thisGroup.getChildNode("groupInfo").getChildNode("name").getText();
            Group g(groupId, groupName);
            listInfo->groups[groupId] = g;
        }
        XMLNode contacts = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("ABFindAllResponse").getChildNode("ABFindAllResult").getChildNode("contacts");

        nItems = contacts.nChildNode("Contact");

        for(int a=0;a<nItems;a++)
        {
            XMLNode thisContact = contacts.getChildNode("Contact", a).getChildNode("contactInfo");
            std::string contactType(thisContact.getChildNode("contactType").getText());

            if(contactType=="Me"){
                // Just grab the current Friendly name
                std::string displayName;
                const char *displayn = thisContact.getChildNode("displayName").getText();
                if (displayn)
                    displayName = displayn;

                listInfo->myDisplayName = displayName;
                continue;
            }
            // TODO - grab all the fields
            // passportName isn't ever present
            std::string passportName;
            std::string contactId;
            const char *passportName_cstr = thisContact.getChildNode("passportName").getText();
            const char *contactId_cstr = contacts.getChildNode("Contact", a).getChildNode("contactId").getText();

            if(passportName_cstr)
                passportName = passportName_cstr;
            else
                continue;

            if(contactId_cstr)
                contactId = contactId_cstr;
            else
                continue;

            if(passportName.empty())
                continue;

            //
            // FIXED 20090823 by José Agustín Terol Sanchis (agus3985@gmail.com):
            //
            // These attributes couldn't be founded at the server response and
            // the method "getText()" will return NULL. This NULL value will
            // throw a exception at the std::string constructor that will
            // abort the address book parsing and, by the way, the connection
            // to the service.
            std::string displayName;
            const char *displayName_cstr = thisContact.getChildNode("displayName").getText();
            if (displayName_cstr)
            {
                displayName = displayName_cstr;
            }
            else
            {
                // We can use the passport name if we haven't received the
                // displayName of the current contact instead of ignore it
                displayName = passportName;
//                 continue;
            }

            std::string isMobileIMEnabled;
            const char *isMobileIMEnabled_cstr = thisContact.getChildNode("isMobileIMEnabled").getText();
            if (isMobileIMEnabled_cstr)
                isMobileIMEnabled = isMobileIMEnabled_cstr;
            else
                continue;  // We could use here a default string such as "false"
        
            std::string isMessengerUser;
            const char *isMessengerUser_cstr = thisContact.getChildNode("isMessengerUser").getText();
            if (isMessengerUser_cstr)
                isMessengerUser = isMessengerUser_cstr;
            else
                continue;  // We could use here a default string such as "true"

            std::string contactType1;
            const char *contactType1_cstr = thisContact.getChildNode("contactType").getText();
            if (contactType1_cstr)
                contactType1 = contactType1_cstr;
            else
                continue; // We could use here a default string such as "Regular"

            transform (passportName.begin(), passportName.end(), passportName.begin(), tolower); 

            // we cant add this contact or we will receive a 205 error
            // see http://trac.adiumx.com/ticket/11126
            if( passportName == "messenger@microsoft.com")
                continue;

            if(!listInfo->contactList[passportName])
                listInfo->contactList[passportName]=new Buddy(passportName,displayName);
            MSN::Buddy *contact = listInfo->contactList[passportName];

            contact->properties["contactId"] = contactId;
            contact->properties["passportName"] = passportName;
            contact->properties["displayName"] = displayName;
            contact->properties["isMobileIMEnabled"] = isMobileIMEnabled;
            contact->properties["isMessengerUser"] = isMessengerUser;
            contact->properties["contactType"] = contactType1;
            contact->userName = contact->properties["passportName"];
            contact->friendlyName = contact->properties["displayName"];
            contact->lists |= MSN::LST_AB;

            XMLNode groupIds = thisContact.getChildNode("groupIds");
            int nItems2 = groupIds.nChildNode("guid");
            for(int b=0;b<nItems2;b++)
            {
                XMLNode guid = groupIds.getChildNode("guid", b);
                std::string groupID(guid.getText());
                listInfo->groups[groupID].buddies.push_back(contact);
                listInfo->contactList[contact->userName]->groups.push_back(&listInfo->groups[groupID]);
            }
        }
        listInfo->progress |= ListSyncInfo::LST_AB;
        std::string lastChange;
        const char *clastChange = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("ABFindAllResponse").getChildNode("ABFindAllResult").getChildNode("ab").getChildNode("lastChange").getText();
        if(clastChange)
            lastChange = clastChange;
        else
            lastChange = "0";

        this->myNotificationServer()->externalCallbacks.gotLatestListSerial(this->myNotificationServer(), lastChange);

        response1.deleteNodeContent();
        this->myNotificationServer()->gotAddressBook(*this);
    }

    void Soap::getOIM(std::string id, bool markAsRead)
    {
        this->oim_id = id;
        this->markAsRead = markAsRead;
        // index 2 is messenger.msn.com
        std::string token = sitesToAuthList[2].BinarySecurityToken;
        std::string t = token.substr(token.find("t=") + 2, token.find("&p=")-2);
        std::string p = token.substr(token.find("&p=") + 3, -1);

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode PassportCookie = XMLNode::createXMLTopNode("PassportCookie");
              PassportCookie.addAttribute("xmlns", "http://www.hotmail.msn.com/ws/2004/09/oim/rsi");
                 XMLNode t1 = XMLNode::createXMLTopNode("t");
             t1.addText(t.c_str());
                 XMLNode p1 = XMLNode::createXMLTopNode("p");
             p1.addText(p.c_str());
              PassportCookie.addChild(t1);
              PassportCookie.addChild(p1);
          header.addChild(PassportCookie);
            envelope.addChild(header);
          XMLNode body = XMLNode::createXMLTopNode("soap:Body");
            XMLNode GetMessage = XMLNode::createXMLTopNode("GetMessage");
              GetMessage.addAttribute("xmlns", "http://www.hotmail.msn.com/ws/2004/09/oim/rsi");
                 XMLNode messageId = XMLNode::createXMLTopNode("messageId");
             messageId.addText(id.c_str());
                 XMLNode alsoMarkAsRead = XMLNode::createXMLTopNode("alsoMarkAsRead");
             if(markAsRead)
                 alsoMarkAsRead.addText("true");
             else
                 alsoMarkAsRead.addText("false");
             GetMessage.addChild(messageId);
             GetMessage.addChild(alsoMarkAsRead);
          body.addChild(GetMessage);
            envelope.addChild(body);

        std::string xml_response;
        char *xml_request = envelope.createXMLString(false);
        std::string temp2 = xml_request;
        this->request_body = temp2;

        requestSoapAction(RETRIEVE_OIM, xml_request, xml_response);

        free(xml_request);
        envelope.deleteNodeContent();
    }

    void Soap::parseGetOIMResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());
        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, RETRIEVE_OIM);
            soapConnection->getOIM(this->oim_id, this->markAsRead);
            return;
        }

        const char* msg = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("GetMessageResponse").getChildNode("GetMessageResult").getText();
        if(msg)
        {
            std::string message1(msg);
            // TODO - extract all the fields. create a struct to carry this data
            message1 = message1.substr(message1.find("\r\n\r\n")+4,-1).c_str();
            std::vector<std::string> a = splitString(message1, "\r\n");
            message1="";
            std::vector<std::string>::iterator i = a.begin();
            for(;i!=a.end(); i++)
            {
                 message1+=(*i);
            }
            message1 = b64_decode(message1.c_str());
            response1.deleteNodeContent();
            // yes, new oim
            this->myNotificationServer()->gotOIM(*this, true, this->oim_id, message1);
            return;
        }
        // no oim with this id
        this->myNotificationServer()->gotOIM(*this, false, this->oim_id, "");
    }

    void Soap::deleteOIM(std::string id)
    {
        this->oim_id=id;
        // index 2 is messenger.msn.com
        std::string token = sitesToAuthList[2].BinarySecurityToken;
        std::string t = token.substr(token.find("t=") + 2, token.find("&p=")-2);
        std::string p = token.substr(token.find("&p=") + 3, -1);

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode PassportCookie = XMLNode::createXMLTopNode("PassportCookie");
              PassportCookie.addAttribute("xmlns", "http://www.hotmail.msn.com/ws/2004/09/oim/rsi");
                 XMLNode t1 = XMLNode::createXMLTopNode("t");
             t1.addText(t.c_str());
                 XMLNode p1 = XMLNode::createXMLTopNode("p");
             p1.addText(p.c_str());
              PassportCookie.addChild(t1);
              PassportCookie.addChild(p1);
          header.addChild(PassportCookie);
            envelope.addChild(header);
          XMLNode body = XMLNode::createXMLTopNode("soap:Body");
            XMLNode DeleteMessages = XMLNode::createXMLTopNode("DeleteMessages");
              DeleteMessages.addAttribute("xmlns", "http://www.hotmail.msn.com/ws/2004/09/oim/rsi");
                 XMLNode messageIds = XMLNode::createXMLTopNode("messageIds");
                   XMLNode messageId = XMLNode::createXMLTopNode("messageId");
                 messageId.addText(id.c_str());
               messageIds.addChild(messageId);
              DeleteMessages.addChild(messageIds);
          body.addChild(DeleteMessages);
            envelope.addChild(body);

        std::string xml_response;
        char *xml_request = envelope.createXMLString(false);
        std::string temp2 = xml_request;
        this->request_body = temp2;

        requestSoapAction(DELETE_OIM, xml_request, xml_response);

        free(xml_request);
        envelope.deleteNodeContent();
    }

    void Soap::parseDeleteOIMResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, DELETE_OIM);
            soapConnection->deleteOIM(this->oim_id);
            return;
        }

        if(http_response_code == "200" )
        {
            this->myNotificationServer()->gotOIMDeleteConfirmation(*this, oim_id, true);
            return;
        }
        this->myNotificationServer()->gotOIMDeleteConfirmation(*this, oim_id, false);
    }

    void Soap::getMailData()
    {
        // index 2 is messenger.msn.com
        std::string token = sitesToAuthList[2].BinarySecurityToken;
        std::string t = token.substr(token.find("t=") + 2, token.find("&p=")-2);
        std::string p = token.substr(token.find("&p=") + 3, -1);

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
          XMLNode header = XMLNode::createXMLTopNode("soap:Header");
            XMLNode PassportCookie = XMLNode::createXMLTopNode("PassportCookie");
              PassportCookie.addAttribute("xmlns", "http://www.hotmail.msn.com/ws/2004/09/oim/rsi");
                 XMLNode t1 = XMLNode::createXMLTopNode("t");
             t1.addText(t.c_str());
                 XMLNode p1 = XMLNode::createXMLTopNode("p");
             p1.addText(p.c_str());
              PassportCookie.addChild(t1);
              PassportCookie.addChild(p1);
          header.addChild(PassportCookie);
            envelope.addChild(header);
          XMLNode body = XMLNode::createXMLTopNode("soap:Body");
            XMLNode GetMetadata = XMLNode::createXMLTopNode("GetMetadata");
              GetMetadata.addAttribute("xmlns", "http://www.hotmail.msn.com/ws/2004/09/oim/rsi");
          body.addChild(GetMetadata);
            envelope.addChild(body);

        std::string xml_response;
        char *xml_request = envelope.createXMLString(false);
        std::string temp2 = xml_request;
        this->request_body = temp2;

        requestSoapAction(RETRIEVE_OIM_MAIL_DATA, xml_request, xml_response);

        free(xml_request);
        envelope.deleteNodeContent();
    }

    void Soap::parseGetMailDataResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301")
        {
            Soap *soapConnection = manageSoapRedirect(response1, RETRIEVE_OIM_MAIL_DATA);
            soapConnection->getMailData();
            return;
        }

        // oh my god! xml text as a field of a xml node! I cant believe it!
        std::string maildata = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("GetMetadataResponse").getChildNode("MD").createXMLString(false);
        if(maildata.empty())
            return; // TODO - raise an error

        response1.deleteNodeContent();
        this->myNotificationServer()->gotSoapMailData(*this, maildata);
    }


    void Soap::generateLockkey(OIM oim)
    {
        //almost equal to send oim

        this->oim = oim;
        // index 4 is messengersecure.live.com
        std::string token = sitesToAuthList[4].BinarySecurityToken;

        // encoding to the required form
        oim.myFname = "=?utf-8?B?" + b64_encode(oim.myFname.c_str(), oim.myFname.length()) + "?=";

        // TODO - find an email library to handle this part
        std::string b64_message = b64_encode(oim.message.c_str(), oim.message.length());
        oim.message="";
        for(unsigned int i=0; i<b64_message.length();i++)
        {
            if( (i%72) == 0 && i != 0 )
            {
                oim.message.append("\r\n");
                oim.message+=b64_message.at(i);
            }
            else
            {
                oim.message += b64_message.at(i);
            }
        }
        std::string msg_body("MIME-Version: 1.0\r\n"
                "Content-Type: text/plain; charset=UTF-8\r\n"
                "Content-Transfer-Encoding: base64\r\n"
                "X-OIM-Message-Type: OfflineMessage\r\n"
                "X-OIM-Run-Id: "+new_branch()+"\r\n"
                "X-OIM-Sequence-Num: 1\r\n\r\n"+
                oim.message);
        oim.full_msg = msg_body;
        this->oim.full_msg = oim.full_msg;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
           XMLNode header = XMLNode::createXMLTopNode("soap:Header");
             XMLNode From = XMLNode::createXMLTopNode("From");
               From.addAttribute("memberName", oim.myUsername.c_str());
               From.addAttribute("friendlyName", oim.myFname.c_str());
               From.addAttribute("xml:lang", "pt-BR");
               From.addAttribute("proxy", "MSNMSGR");
               From.addAttribute("xmlns", "http://messenger.msn.com/ws/2004/09/oim/");
               From.addAttribute("msnpVer", "MSNP15");
               From.addAttribute("buildVer", "8.1.0178");
             XMLNode To = XMLNode::createXMLTopNode("To");
               To.addAttribute("memberName", oim.toUsername.c_str());
               To.addAttribute("xmlns", "http://messenger.msn.com/ws/2004/09/oim/");
             XMLNode Ticket = XMLNode::createXMLTopNode("Ticket");
               Ticket.addAttribute("passport", decodeURL(token).c_str());
               Ticket.addAttribute("appid", szClientID);
               Ticket.addAttribute("lockkey", "");
               Ticket.addAttribute("xmlns", "http://messenger.msn.com/ws/2004/09/oim/");
              XMLNode Sequence = XMLNode::createXMLTopNode("Sequence");
               Sequence.addAttribute("xmlns", "http://schemas.xmlsoap.org/ws/2003/03/rm");
            XMLNode Identifier = XMLNode::createXMLTopNode( "Identifier" );
            Identifier.addAttribute("xmlns", "http://schemas.xmlsoap.org/ws/2002/07/utility");
            Identifier.addText("http://messenger.msn.com");
              XMLNode MessageNumber = XMLNode::createXMLTopNode("MessageNumber");
            MessageNumber.addText("1");
              Sequence.addChild(Identifier);
              Sequence.addChild(MessageNumber);
           header.addChild(From);
           header.addChild(To);
           header.addChild(Ticket);
           header.addChild(Sequence);
        envelope.addChild(header);
           XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
             XMLNode MessageType = XMLNode::createXMLTopNode( "MessageType" );
               MessageType.addAttribute("xmlns","http://messenger.msn.com/ws/2004/09/oim/");
               MessageType.addText("text");
             XMLNode Content = XMLNode::createXMLTopNode( "Content" );
               Content.addAttribute("xmlns","http://messenger.msn.com/ws/2004/09/oim/");
             Content.addText(oim.full_msg.c_str());
           body.addChild(MessageType);
           body.addChild(Content);
        envelope.addChild(body);

        std::string xml_response;
        char *xml_request = envelope.createXMLString(false);
        std::string temp2 = xml_request;
        this->request_body = temp2;

        requestSoapAction(GENERATE_LOCKKEY, xml_request, xml_response);

        free(xml_request);
        envelope.deleteNodeContent();
    }

    void Soap::parseGenerateLockkeyResponse(std::string response)
    {
        OIM oim = this->oim;
        // probably we need to generate a new lockkey
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301")
        {
            Soap *soapConnection = manageSoapRedirect(response1, GENERATE_LOCKKEY);
            soapConnection->generateLockkey(this->oim);
            return;
        }

        XMLNode LockKeyChallenge = response1.getChildNode("soap:Envelope").getChildNode("soap:Body").getChildNode("soap:Fault").getChildNode("detail").getChildNode("LockKeyChallenge");

        const char * lockkey1 = LockKeyChallenge.getText();
        if(!lockkey1)
        {
            this->myNotificationServer()->gotOIMLockkey(*this, "");
            return;
        }
        std::string lockkey = lockkey1;
        char b[33];
        memset(&b,0,33);
        DoMSNP11Challenge(lockkey.c_str(),b);
        std::string new_lockkey(b);
        this->lockkey = new_lockkey;

        this->myNotificationServer()->gotOIMLockkey(*this, this->lockkey);
    }

    void Soap::sendOIM(OIM oim, std::string lockkey)
    {
        this->oim = oim;
        this->lockkey = lockkey;
        // index 4 is messengersecure.live.com
        std::string token = sitesToAuthList[4].BinarySecurityToken;

        // encoding to the required form
        oim.myFname = "=?utf-8?B?" + b64_encode(oim.myFname.c_str(), oim.myFname.length()) + "?=";

        // TODO - find an email library to handle this part
        std::string b64_message = b64_encode(oim.message.c_str(), oim.message.length());
        oim.message="";
        for(unsigned int i=0; i<b64_message.length();i++)
        {
            if( (i%72) == 0 && i != 0 )
            {
                oim.message.append("\r\n");
                oim.message+=b64_message.at(i);
            }
            else
            {
                oim.message += b64_message.at(i);
            }
        }
        std::string msg_body("MIME-Version: 1.0\r\n"
                "Content-Type: text/plain; charset=UTF-8\r\n"
                "Content-Transfer-Encoding: base64\r\n"
                "X-OIM-Message-Type: OfflineMessage\r\n"
                "X-OIM-Run-Id: "+new_branch()+"\r\n"
                "X-OIM-Sequence-Num: 1\r\n\r\n"+
                oim.message);
        oim.full_msg = msg_body;
        this->oim.full_msg = oim.full_msg;

        XMLNode envelope = XMLNode::createXMLTopNode("soap:Envelope");
        envelope.addAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd", "http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soap", "http://schemas.xmlsoap.org/soap/envelope/");
           XMLNode header = XMLNode::createXMLTopNode("soap:Header");
             XMLNode From = XMLNode::createXMLTopNode("From");
               From.addAttribute("memberName", oim.myUsername.c_str());
               From.addAttribute("friendlyName", oim.myFname.c_str());
               From.addAttribute("xml:lang", "pt-BR");
               From.addAttribute("proxy", "MSNMSGR");
               From.addAttribute("xmlns", "http://messenger.msn.com/ws/2004/09/oim/");
               From.addAttribute("msnpVer", "MSNP15");
               From.addAttribute("buildVer", "8.1.0178");
             XMLNode To = XMLNode::createXMLTopNode("To");
               To.addAttribute("memberName", oim.toUsername.c_str());
               To.addAttribute("xmlns", "http://messenger.msn.com/ws/2004/09/oim/");
             XMLNode Ticket = XMLNode::createXMLTopNode("Ticket");
               Ticket.addAttribute("passport", decodeURL(token).c_str());
               Ticket.addAttribute("appid", szClientID);
               Ticket.addAttribute("lockkey", lockkey.c_str());
               Ticket.addAttribute("xmlns", "http://messenger.msn.com/ws/2004/09/oim/");
              XMLNode Sequence = XMLNode::createXMLTopNode("Sequence");
               Sequence.addAttribute("xmlns", "http://schemas.xmlsoap.org/ws/2003/03/rm");
            XMLNode Identifier = XMLNode::createXMLTopNode( "Identifier" );
            Identifier.addAttribute("xmlns", "http://schemas.xmlsoap.org/ws/2002/07/utility");
            Identifier.addText("http://messenger.msn.com");
              XMLNode MessageNumber = XMLNode::createXMLTopNode("MessageNumber");
            MessageNumber.addText("1");
              Sequence.addChild(Identifier);
              Sequence.addChild(MessageNumber);
           header.addChild(From);
           header.addChild(To);
           header.addChild(Ticket);
           header.addChild(Sequence);
        envelope.addChild(header);
           XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
             XMLNode MessageType = XMLNode::createXMLTopNode( "MessageType" );
               MessageType.addAttribute("xmlns","http://messenger.msn.com/ws/2004/09/oim/");
               MessageType.addText("text");
             XMLNode Content = XMLNode::createXMLTopNode( "Content" );
               Content.addAttribute("xmlns","http://messenger.msn.com/ws/2004/09/oim/");
             Content.addText(oim.full_msg.c_str());
           body.addChild(MessageType);
           body.addChild(Content);
        envelope.addChild(body);

        std::string xml_response;
        char *xml_request = envelope.createXMLString(false);
        std::string temp2 = xml_request;
        this->request_body = temp2;

        requestSoapAction(SEND_OIM, xml_request, xml_response);

        free(xml_request);
        envelope.deleteNodeContent();
    }

    void Soap::parseSendOIMResponse(std::string response)
    {
        OIM oim = this->oim;
        XMLNode response1 = XMLNode::parseString(response.c_str());

        if(http_response_code == "301")
        {
            Soap *soapConnection = manageSoapRedirect(response1, SEND_OIM);
            soapConnection->sendOIM(this->oim, this->lockkey);
            return;
        }

        if(http_response_code == "200" )
        {
            this->myNotificationServer()->gotOIMSendConfirmation(*this, oim.id, true);
            return;
        }
        this->myNotificationServer()->gotOIMSendConfirmation(*this, oim.id, false);
    }

    void Soap::changeDisplayName(std::string newDisplayName)
    {
        this->tempDisplayName = newDisplayName;
        XMLNode envelope = XMLNode::createXMLTopNode( "soap:Envelope" );
        envelope.addAttribute("xmlns:soap","http://schemas.xmlsoap.org/soap/envelope/");
        envelope.addAttribute("xmlns:xsi","http://www.w3.org/2001/XMLSchema-instance");
        envelope.addAttribute("xmlns:xsd","http://www.w3.org/2001/XMLSchema");
        envelope.addAttribute("xmlns:soapenc","http://schemas.xmlsoap.org/soap/encoding/");
         XMLNode header = XMLNode::createXMLTopNode("soap:Header");
           XMLNode ABApplicationHeader = XMLNode::createXMLTopNode( "ABApplicationHeader" );
           ABApplicationHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
             XMLNode ApplicationId = XMLNode::createXMLTopNode( "ApplicationId" );
               ApplicationId.addText("996CDE1E-AA53-4477-B943-2BE802EA6166");
             XMLNode IsMigration = XMLNode::createXMLTopNode("IsMigration");
               IsMigration.addText("false");
             XMLNode PartnerScenario = XMLNode::createXMLTopNode("PartnerScenario");
               PartnerScenario.addText("Timer");
           ABApplicationHeader.addChild(ApplicationId);
           ABApplicationHeader.addChild(IsMigration);
           ABApplicationHeader.addChild(PartnerScenario);

           XMLNode ABAuthHeader = XMLNode::createXMLTopNode("ABAuthHeader");
             ABAuthHeader.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
             XMLNode ManagedGroupRequest = XMLNode::createXMLTopNode("ManagedGroupRequest");
               ManagedGroupRequest.addText("false");
             XMLNode TicketToken = XMLNode::createXMLTopNode("TicketToken");
               TicketToken.addText(sitesToAuthList[3].BinarySecurityToken.c_str());
            ABAuthHeader.addChild(ManagedGroupRequest);
            ABAuthHeader.addChild(TicketToken);

          header.addChild(ABApplicationHeader);
          header.addChild(ABAuthHeader);
        envelope.addChild(header);
         XMLNode body = XMLNode::createXMLTopNode( "soap:Body" );
           XMLNode ABContactUpdate = XMLNode::createXMLTopNode( "ABContactUpdate" );
             ABContactUpdate.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
               XMLNode abId = XMLNode::createXMLTopNode( "abId" );
                 abId.addText("00000000-0000-0000-0000-000000000000");
               XMLNode contacts = XMLNode::createXMLTopNode( "contacts" );
                 XMLNode Contact = XMLNode::createXMLTopNode( "Contact" );
                   Contact.addAttribute("xmlns","http://www.msn.com/webservices/AddressBook");
                     XMLNode contactInfo = XMLNode::createXMLTopNode( "contactInfo" );
                       XMLNode contactType = XMLNode::createXMLTopNode( "contactType" );
                           contactType.addText("Me");
                       XMLNode displayName = XMLNode::createXMLTopNode( "displayName" );
                           displayName.addText(newDisplayName.c_str());
                   contactInfo.addChild(contactType);
                 contactInfo.addChild(displayName);
                     XMLNode propertiesChanged = XMLNode::createXMLTopNode( "propertiesChanged" );
                 propertiesChanged.addText("DisplayName");
                  Contact.addChild(contactInfo);
                  Contact.addChild(propertiesChanged);
                contacts.addChild(Contact);
              ABContactUpdate.addChild(abId);
              ABContactUpdate.addChild(contacts);
               body.addChild(ABContactUpdate);
            envelope.addChild(body);

        std::string xml_response;
        char *xml_request = envelope.createXMLString(false);
        std::string temp2 = xml_request;
        this->request_body = temp2;

        requestSoapAction(CHANGE_DISPLAYNAME, xml_request, xml_response);

        free(xml_request);
        envelope.deleteNodeContent();

    }

    void Soap::parseChangeDisplayNameResponse(std::string response)
    {
        XMLNode response1 = XMLNode::parseString(response.c_str());
        if(http_response_code == "301" )
        {
            Soap *soapConnection = manageSoapRedirect(response1, CHANGE_DISPLAYNAME);
            soapConnection->changeDisplayName(this->tempDisplayName);
            return;
        }

        if(http_response_code == "200" )
        {
            this->myNotificationServer()->gotChangeDisplayNameConfirmation(*this, this->tempDisplayName, true);
            return;
        }
        this->myNotificationServer()->gotChangeDisplayNameConfirmation(*this, this->tempDisplayName, false);
    }

    void Soap::socketConnectionCompleted()
    {
        Connection::socketConnectionCompleted();
           this->myNotificationServer()->externalCallbacks.unregisterSocket(this->sock);
           this->myNotificationServer()->externalCallbacks.registerSocket(this->sock, 1, 0, true);
    }

    void Soap::handleIncomingData()
    {
        // grab http header
        if(this->http_header_response.empty())
        {
            if (this->readBuffer.find("\r\n\r\n") == std::string::npos )
                return;

            http_header_response = this->readBuffer.substr(0,this->readBuffer.find("\r\n\r\n") + 4);
            Message::Headers headers = Message::Headers(http_header_response);
            this->response_length = decimalFromString(headers["Content-Length"]);

            this->http_response_code = splitString(http_header_response.substr(0,http_header_response.find("\r\n"))," ")[1];

            // drop http from buffer
            this->readBuffer = this->readBuffer.substr(this->readBuffer.find("\r\n\r\n") + 4);
        }

        if(this->readBuffer.length() < this->response_length)
            return; // wait for the full response
        
        this->response_body = this->readBuffer;

        this->readBuffer.clear();

        this->myNotificationServer()->externalCallbacks.unregisterSocket(this->sock);

        switch(this->action)
        {
            case AUTH:
                parseGetTicketsResponse(this->response_body);
                break;
            case GET_LISTS:
                parseGetListsResponse(this->response_body);
                break;
            case GET_ADDRESS_BOOK:
                parseGetAddressBookResponse(this->response_body);
                break;
            case SEND_OIM:
                parseSendOIMResponse(this->response_body);
                break;
            case GENERATE_LOCKKEY:
                parseGenerateLockkeyResponse(this->response_body);
                break;
            case RETRIEVE_OIM:
                parseGetOIMResponse(this->response_body);
                break;
            case DELETE_OIM:
                parseDeleteOIMResponse(this->response_body);
                break;
            case RETRIEVE_OIM_MAIL_DATA:
                parseGetMailDataResponse(this->response_body);
                break;
            case ADD_CONTACT_TO_LIST:
                parseAddContactToListResponse(this->response_body);
                break;
            case DEL_CONTACT_FROM_LIST:
                parseRemoveContactFromListResponse(this->response_body);
                break;
            case CHANGE_DISPLAYNAME:
                parseChangeDisplayNameResponse(this->response_body);
                break;
            case ADD_CONTACT_TO_GROUP:
                parseAddContactToGroupResponse(this->response_body);
                break;
            case DEL_CONTACT_FROM_GROUP:
                parseDelContactFromGroupResponse(this->response_body);
                break;
            case ADD_GROUP:
                parseAddGroupResponse(this->response_body);
                break;
            case DEL_GROUP:
                parseDelGroupResponse(this->response_body);
                break;
            case RENAME_GROUP:
                parseRenameGroupResponse(this->response_body);
                break;
            case DISABLE_CONTACT_ON_ADDRESSBOOK:
                parseDisableContactFromAddressBookResponse(this->response_body);
                break;
            case ENABLE_CONTACT_ON_ADDRESSBOOK:
                parseEnableContactOnAddressBookResponse(this->response_body);
                break;
            case ADD_CONTACT_TO_ADDRESSBOOK:
                parseAddContactToAddressBookResponse(this->response_body);
                break;
            case DEL_CONTACT_FROM_ADDRESSBOOK:
                parseDelContactFromAddressBookResponse(this->response_body);
                break;

        }
        delete this;
    }

    Soap* Soap::manageSoapRedirect(XMLNode response1, soapAction action)
    {
        Soap *soapConnection = new Soap(notificationServer, sitesToAuthList);
        Message::Headers headers = Message::Headers(http_header_response);
        std::string newdomain;
        std::string location = headers["Location"];

        const char *preferredHostName = response1.getChildNode("soap:Envelope").getChildNode("soap:Header").getChildNode("ServiceHeader").getChildNode("PreferredHostName").getText();
        if(preferredHostName)
        {
            std::string newdomain(preferredHostName);
            soapConnection->actionDomains[action] = newdomain;
        }

        if (location.size())
        {
            std::string newurl1(location);
            std::vector<std::string> a = splitString(newurl1, "/");
            std::string newdomain = splitString(a[1], "/")[0];
            soapConnection->actionDomains[action] = newdomain;
            std::vector<std::string> postpath = splitString(newurl1, newdomain);
            soapConnection->actionPOSTURLs[action] = postpath[1];
        }
        soapConnection->setMBI(mbi);

        return soapConnection;
    }

    void Soap::disconnect()
    {
    }

    Soap::~Soap()
    {
        Connection::disconnect();
        if(this->myNotificationServer()->connectionState() != MSN::NotificationServerConnection::NS_DISCONNECTED)
            this->myNotificationServer()->removeSoapConnection(this);
    }
}
