#ifndef __msn_passport_h__
#define __msn_passport_h__

/*
 * passport.h
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

#include <string>
#include <stdexcept>
#include <iostream>

#include "libmsn_export.h"

namespace MSN
{
    /** An InvalidPassport exception will be thrown whenever
     *  a malformed passport is passed to a function that requires
     *  a valid address.
     */
    class InvalidPassport : public std::runtime_error
    {
public:
        InvalidPassport(std::string err) : std::runtime_error(err) {};
    };
    
    /** A Passport represents a passport address.  It is used to
     *  validate these addresses for functions that require it.
     *
     *  @todo Document validation rules.
     *  @todo Investigate subclassing std::string to reduce code duplication.
     */
    class LIBMSN_EXPORT Passport
    {
public:
        Passport(std::string email_) : email(email_) { validate(); };
        Passport(const char *email_) : email(std::string(email_)) { validate(); };
        Passport() : email("") {};
        
        operator std::string() const;
        const char *c_str() const;
        bool operator ==(const Passport & other) const { return this->email == other.email; };
        
        friend bool operator ==(const Passport & p, const std::string & other) { return p.email == other; };
        friend bool operator ==(const std::string & other, const Passport & p) { return p.email == other; };
        friend std::istream& operator >>(std::istream & is, Passport & p) { is >> p.email; p.validate(); return is; }
        friend std::ostream& operator <<(std::ostream & os, Passport & p) { os << p.email; p.validate(); return os; }
private:
        void validate();
        std::string email;
    };
}

std::ostream & operator << (std::ostream & os, const MSN::Passport& passport);
#endif
