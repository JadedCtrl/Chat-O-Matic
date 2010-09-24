#ifndef __msn_msnobj_h__
#define __msn_msnobj_h__

/*
 * msnobject.h
 * libmsn
 *
 * Created by Tiago Salem Herrmann on Mon Ago 22 2007.
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

#include "libmsn_export.h"

namespace MSN
{
    class LIBMSN_EXPORT MSNObject
    {
private:
        unsigned int current_id;
        std::string Creator;
        typedef struct 
        {
            std::string Creator;
            unsigned long long Size;
            int Type;
            std::string Location;
            std::string realLocation;
            std::string Friendly;
            std::string SHA1D;
            std::string SHA1C;
            std::string XMLString;
        } MSNObjectUnit;

        std::list<MSNObjectUnit> msnObjects;
public:
        MSNObject() : current_id(0) {};
        ~MSNObject() {};
        void setCreator(std::string creator);
        void addMSNObject(std::string filename, int Type);
        bool getMSNObjectXML(std::string filename, int Type, std::string & msnobj);
        bool getMSNObject(std::string filename, int Type, MSNObjectUnit & msnobj);
        bool delMSNObjectByType(int Type);
        bool getMSNObjectXMLByType(int Type, std::string & xml);
        bool getMSNObjectRealPath(std::string xml, std::string & realpath);
    };
}
#endif
