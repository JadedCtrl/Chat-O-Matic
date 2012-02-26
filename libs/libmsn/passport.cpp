/*
 * passport.cpp
 * libmsn
 *
 * Created by Mark Rowe on Thu May 20 2004.
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

#include <passport.h>
#include <stdio.h>

namespace MSN 
{
    void Passport::validate()
    {
        if (email.find(" ") != std::string::npos)
            throw InvalidPassport("Passport must not contain any spaces!");
        
        if (email.find("@") == std::string::npos || email.find("@") != email.rfind("@"))
            throw InvalidPassport("Passport must contain exactly one '@' character!");
        
        if (email.find("@") == 0)
            throw InvalidPassport("Passport must have at least one character before the '@'!");
        
        if (email.find(".", email.find("@")) == std::string::npos)
            throw InvalidPassport("Passport must have at least one '.' after the '@'!");
        
        if (email.find(".", email.find("@")) - email.find("@") < 2)
            throw InvalidPassport("Passport must have at least one character between the '@' and the '.'!");
        
        if (email[email.size() - 1] == '.')
            throw InvalidPassport("Passport must not end with a '.' character!");
        
        if (email.size() < 5)
            throw InvalidPassport("Passport must contain at least 5 characters!");
    }
        
    Passport::operator std::string() const
    {
        return email;
    }
    
    const char *Passport::c_str() const
    {
        return email.c_str();
    }
}

std::ostream & operator <<(std::ostream & os, const MSN::Passport & passport)
{
    return os << std::string(passport);
}
