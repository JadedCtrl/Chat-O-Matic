/*
 * buddy.cpp
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

#include "buddy.h"
#include <cassert>

namespace MSN
{
    std::string buddyStatusToString(BuddyStatus state)
    {
        switch (state)
        {
            case STATUS_AVAILABLE:
                return "NLN";
            case STATUS_BUSY:
                return "BSY";
            case STATUS_IDLE:
                return "IDL";
            case STATUS_BERIGHTBACK:
                return "BRB";
            case STATUS_AWAY:
                return "AWY";
            case STATUS_ONTHEPHONE:
                return "PHN";
            case STATUS_OUTTOLUNCH:
            return "LUN";
            case STATUS_INVISIBLE:
                return "HDN";
            default:
                assert(false);
        }
    }
    
    BuddyStatus buddyStatusFromString(std::string state)
    {
        if (state == "NLN")
            return STATUS_AVAILABLE;
        else if (state == "BSY")
            return STATUS_BUSY;
        else if (state == "IDL")
            return STATUS_IDLE;
        else if (state == "BRB")
            return STATUS_BERIGHTBACK;
        else if (state == "AWY")
            return STATUS_AWAY;
        else if (state == "PHN")
            return STATUS_ONTHEPHONE;
        else if (state == "LUN")
            return STATUS_OUTTOLUNCH;
        else if (state == "HDN")
            return STATUS_INVISIBLE;
        else
            throw std::runtime_error("Unknown status!");
    }
}
