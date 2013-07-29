#ifndef __msn_buddy_h__
#define __msn_buddy_h__

/*
 * buddy.h
 * libmsn
 *
 * Created by Mark Rowe on Mon Apr 19 2004.
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
#include <passport.h>

#include "libmsn_export.h"

namespace MSN
{
    /** The online state of a buddy.
    */
    
    enum BuddyStatus
    {
        STATUS_AVAILABLE, /**< Contact is available */
        STATUS_BUSY, /**< Contact is busy */
        STATUS_IDLE, /**< Contact is idle */
        STATUS_BERIGHTBACK, /**< Contact will be right back */
        STATUS_AWAY, /**< Contact is away */
        STATUS_ONTHEPHONE, /**< Contact is on the phone */
        STATUS_OUTTOLUNCH, /**< Contact is out to lunch */
        STATUS_INVISIBLE /**< Contact is invisible */
    };
    
    std::string LIBMSN_EXPORT buddyStatusToString(BuddyStatus s);
    BuddyStatus LIBMSN_EXPORT buddyStatusFromString(std::string s);
    
    class Group;
    
    /** The Buddy class contains information about a member of a buddy list.
     *
     *  Each Buddy is made up of their passport address (@a userName),
     *  user-visible display name (@a friendlyName), a list of properties 
     *  (@a properties) and zero or more @a groups on the buddy list that they belong to.
     *
     */
    class LIBMSN_EXPORT Buddy
    {
public:
        /** The PhoneNumbers class contains information about one or more phone numbers
         *  that are retrieved during the buddy list synchronisation process. 
         */
        class PhoneNumber
        {
public:
            /** The name of this phone number.
             *
             *  @todo Should this be an enumeration containing the possible
             *        types of phone number?
             */
             std::string title;

             std::string number;

             bool enabled;
 
              PhoneNumber(std::string title_, std::string number_, bool enabled_=true)
                 : title(title_), number(number_), enabled(enabled_) {};

        };

        /** all the properties received at login time */
        std::map<std::string, std::string> properties;
       
        /** Their passport address */
        Passport userName;
        
        /** Their friendly name */
        std::string friendlyName;

        /** A list of phone numbers related to this buddy */
        std::list<Buddy::PhoneNumber> phoneNumbers;

        /** A list of Group's that this buddy is a member of */
        std::list<Group *> groups;

        /** Lists which this contact belong. Pending, Forward, Block... **/
        unsigned int lists;
        
        Buddy(Passport userName_, std::string friendlyName_ = "") :
            userName(userName_), friendlyName(friendlyName_), lists(0) {};
        bool operator==(const Buddy &other) { return userName == other.userName; }
    };
    
    /** The Group class represents a group of contacts on the buddy list.
     *
     *  Each group is represented by a @a groupID, a list of buddies @buddies
     *  and has a user-visible @a name.
     */
    class LIBMSN_EXPORT Group
    {
public:
    
        /** Id of this group **/
        std::string groupID;

        /** Name of this group **/
        std::string name;

        /** List of contacts in this group **/
        std::list<Buddy *> buddies;
        
        Group(std::string groupID_, std::string name_)
            : groupID(groupID_), name(name_) {};
        
        Group() : name("INVALID") {};
    };
}

#endif
