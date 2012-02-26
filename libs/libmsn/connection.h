#ifndef __msn_connection_h__
#define __msn_connection_h__

/*
 * connection.h
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
#include <string>
#include <list>
#include <vector>
#include <map>
#include <stdexcept>

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

#include "libmsn_export.h"

namespace MSN
{
    class callback;
    class Message;
    class Passport;
    class NotificationServerConnection;
    
    /** An abstract base class that represents a connection to another computer.
     *
     *  Connection provides an interface and some functionality that is common
     *  to notification, switchboard and file transfer connections.
     *
     */
    class LIBMSN_EXPORT Connection
    {      
public:
        /** The socket which connects this Connection to another computer
         *
         * @deprecated  In the future, this member will be made private.  Any
         *              functions that access this member should be converted to
         *              member functions of a subclass.
         */
        void *sock;
        
        /** Indicates whether a connection has been established.
         */
        bool connected;

        
protected:
        
        std::string readBuffer;
public:
        /** The transaction ID of the next command to be sent.
         */
        int trID;

        Connection();
        virtual ~Connection();
        
        /** Dispatch a command to its appropriate handler routines based on @a args.
         * 
         *  @param  args  A vector of strings containing arguments, returned from readLine.
         */
        virtual void dispatchCommand(std::vector<std::string> & args) = 0;
        
        /** Read a line from the network and split it into its components.
         *
         *  MSN commands and their arguments are separated by white space.
         */
        std::vector<std::string> getLine();
        
        bool isWholeLineAvailable();
        bool bytesAvailable();
        
        /** Write a string to the connection.
         *
         */
        virtual size_t write(std::string s, bool log=true) throw (std::runtime_error);
        
        /** Write the contents of a stringstream to the connection.
         *    
         * @param  s    The stringstream to write.
         * @param  log  Should we log this output to the console.
         *
         * write will buffer the output if a connection has not yet been established.
         * In this case, the data will be written as soon as a connection is 
         * established.
         */
        virtual size_t write(std::ostringstream & s, bool log=true) throw (std::runtime_error);
                
        /** Connect ourself to @a hostname on @a port.
         */
        virtual void connect(const std::string & hostname, unsigned int port) = 0;
        virtual void disconnect() = 0;
        
        /** @name External Socket Hooks
         *  
         *  These members should be called whenever an appropriate socket event
         *  occurs.
         */
        /** @{ */
        
        /** New data is available on the connection.
         */
        virtual void dataArrivedOnSocket();
        
        /** The connection has been established.
         */
        virtual void socketConnectionCompleted();
        
        virtual void socketIsWritable() {};
        
        /** An error has occurred on the socket.
         */
        virtual void errorOnSocket(int errno_);
        /** @} */
                
        /** Notify the calling library that an error with code @a errorCode has
         *  occured.
         */
        void showError(int errorCode);

        /** Is this Connection connected to a remote endpoint?
        */
        bool isConnected() { return this->connected; };
        virtual NotificationServerConnection *myNotificationServer() = 0;        

protected:
        virtual void handleIncomingData() = 0;
private:
        std::string writeBuffer;
    };
}
#endif
