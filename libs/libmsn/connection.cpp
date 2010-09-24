/*
 * connection.cpp
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

#include "connection.h"
#include "errorcodes.h"
#include "util.h"
#include "passport.h"
#include "externals.h"
#include "notificationserver.h"
#ifndef WIN32
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#else
#include <winsock.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <cerrno>
#include <time.h>
#include <cassert>

namespace MSN
{
    static std::vector<std::string> errors;

    Connection::Connection() 
        : sock(NULL), connected(false), trID(1)
    {
        srand((unsigned int) time(NULL));

        if (errors.size() != 0)
        {
            assert(errors.size() == 1000);
        }
        else
        {
            errors.resize(1000);
            for (int a = 0; a < 1000; a++)
            {
                errors[a] = "Unknown error code";
            }
            
            errors[200] = "Syntax error";
            errors[201] = "Invalid parameter";
            errors[205] = "Invalid user";
            errors[206] = "Domain name missing from username";
            errors[207] = "Already logged in";
            errors[208] = "Invalid username";
            errors[209] = "Invalid friendly name";
            errors[210] = "List full";
            errors[215] = "This user is already on this list or in this session";
            errors[216] = "Not on list";
            errors[218] = "Already in this mode";
            errors[219] = "This user is already in the opposite list";
            errors[241] = "Unable to add user";
            errors[280] = "Switchboard server failed";
            errors[281] = "Transfer notification failed";
            errors[300] = "Required fields missing";
            errors[302] = "Not logged in";
            errors[500] = "Internal server error";
            errors[501] = "Database server error";
            errors[510] = "File operation failed at server";
            errors[520] = "Memory allocation failed on server";
            errors[600] = "The server is too busy";
            errors[601] = "The server is unavailable";
            errors[602] = "A Peer Notification Server is down";
            errors[603] = "Database connection failed";
            errors[604] = "Server going down for maintenance";
            errors[707] = "Server failed to create connection";
            errors[711] = "Blocking write failed on server";
            errors[712] = "Session overload on server";
            errors[713] = "You have been too active recently. Slow down!";
            errors[714] = "Too many sessions open";
            errors[715] = "Email Address Not verified";
            errors[717] = "Bad friend file on server";
            errors[911] = "Authentication failed. Check that you typed your username and password correctly.";
            errors[913] = "This action is not allowed while you are offline";
            errors[920] = "This server is not accepting new users";            
            errors[921] = "Error synchronizing lists";
            errors[922] = "Error synchronizing address book";
        }

    }
    
    Connection::~Connection() { }
    
    void Connection::disconnect()
    {
        this->connected = false;
        this->myNotificationServer()->externalCallbacks.unregisterSocket(this->sock);

        this->myNotificationServer()->externalCallbacks.closeSocket(this->sock);

        this->sock = NULL;
        this->writeBuffer.erase();
        this->readBuffer.erase();
        this->trID = 1;
    }
    
    std::vector<std::string> Connection::getLine()
    {
        assert(this->isWholeLineAvailable());
        std::string s = this->readBuffer.substr(0, this->readBuffer.find("\r\n"));
        this->myNotificationServer()->externalCallbacks.log(0, (s + "\n").c_str());
        return splitString(s, " ");
    }
    
    bool Connection::isWholeLineAvailable()
    {
        return this->readBuffer.find("\r\n") != std::string::npos;
    }
    
    void Connection::errorOnSocket(int errno_)
    {
        this->myNotificationServer()->externalCallbacks.showError(this, strerror(errno_));
        this->disconnect();
    }
    
    void Connection::socketConnectionCompleted()
    {
        this->connected = true;

        if(this->writeBuffer.size())
        {
            // We know that we are connected, so this will try writing to the network.
            size_t writtenLength = this->write(this->writeBuffer, 1);
            if(writtenLength > 0 && this->writeBuffer.size() > 0)
                this->writeBuffer = this->writeBuffer.substr(writtenLength);
        }
    }
        
    size_t Connection::write(std::string s, bool log) throw (std::runtime_error)
    {
        if(s.size() < 0)
            return 0;

        if (! this->connected)
        {
            this->writeBuffer.append(s);
        }
        else
        {
            if (log)
                this->myNotificationServer()->externalCallbacks.log(1, s.c_str());

            char *a = (char*)s.c_str();
            size_t written = this->myNotificationServer()->
                        externalCallbacks.writeDataToSocket(sock, a, (int) (s.size()));
            return written;
        }
        return s.size();
    }
    
    size_t Connection::write(std::ostringstream & ss, bool log) throw (std::runtime_error)
    {
        std::string s = ss.str();
#ifdef DEBUG
        std::cout << s << std::endl;
#endif
        size_t result = write(s, log);
        return result;
    }
 
    void Connection::dataArrivedOnSocket()
    {
        char tempReadBuffer[8192];
        int amountRead = 8192;
        std::string tempRead;
        while (amountRead == 8192)
        {
            amountRead = this->myNotificationServer()->externalCallbacks.getDataFromSocket(sock, tempReadBuffer, 8192);
            if(amountRead < 0)
                break;
            tempRead+= std::string(tempReadBuffer,amountRead);
        }
        if (tempRead.length() < 0)
        {
            // We shouldn't be here because dataArrivedOnSocket
            // is only called when select/poll etc has told us that
            // the socket is readable.
            // assert(errno != EAGAIN);
            this->myNotificationServer()->externalCallbacks.showError(this, "No data to read");
            this->disconnect();
        }
        else if (amountRead == 0)
        {
            this->myNotificationServer()->externalCallbacks.showError(this, "Connection closed by remote endpoint.");
            this->disconnect();
        }
        else
        {
            this->readBuffer += tempRead;
#ifdef DEBUG
            std::cout << tempRead << std::endl;
#endif
            try
            {
                handleIncomingData();
            }
            catch (std::exception & e)
            {
                this->myNotificationServer()->externalCallbacks.showError(this, e.what());
            }
        }
    }

    void Connection::showError(int errorCode)
    {
        std::ostringstream buf_;
        buf_ << "Error code: " << errorCode << " (" << errors[errorCode] << ")";
        this->myNotificationServer()->externalCallbacks.showError(this, buf_.str());
    }
}
