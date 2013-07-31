/*
 * msnobject.cpp
 * libmsn
 *
 * Created by Tiago Salem Herrmann on 08/2007.
 * Copyright (c) 2007 Tiago Salem Herrmann. All rights reserved
 
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
#include <util.h>
#include <openssl/sha.h>
#include <msnobject.h>
#include <xmlParser.h>
#include <iostream>
#include <fstream>

namespace MSN
{
    void MSNObject::addMSNObject(std::string filename, int Type)
    {
        std::streampos size;
        char * memblock;
        SHA_CTX  ctx;
        unsigned char digest[SHA_DIGEST_LENGTH];
        
        MSNObjectUnit msnobj;
        msnobj.Creator = this->Creator;
        msnobj.Size = FileSize(filename.c_str());
        msnobj.Type = Type;

        if(Type!=11)
        {
            msnobj.Location = toStr(++current_id);
            msnobj.Location += ".tmp";
        }
        else
        {
            msnobj.Location = "0";
            // encode wav to siren
            libmsn_Siren7_EncodeVoiceClip(filename);
            msnobj.Size = FileSize(filename.c_str());
        }

        msnobj.realLocation = filename;
        if(Type!=5 && Type!=8)
            msnobj.Friendly="AAA=";

        std::ifstream file (filename.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
        if (!file.is_open())
        {
            return;
        }

        size = file.tellg();
        memblock = new char [size];
        file.seekg (0, std::ios::beg);
        file.read (memblock, size);
        file.close();

        SHA1_Init(&ctx);
        SHA1_Update(&ctx, memblock, size);
        SHA1_Final(digest, &ctx);

        delete[] memblock;

        msnobj.SHA1D = b64_encode((const char*)digest,20);
        std::string all_fields("Creator" + msnobj.Creator + 
                "Size" + toStr(msnobj.Size) +
                "Type" + toStr(msnobj.Type) +
                "Location" + msnobj.Location +
                "Friendly" + msnobj.Friendly +
                "SHA1D" + msnobj.SHA1D);

        SHA1_Init(&ctx);
        SHA1_Update(&ctx, all_fields.c_str(), all_fields.length());
        SHA1_Final(digest, &ctx);
        msnobj.SHA1C = b64_encode((const char*)digest,20);
        XMLNode msnObject = XMLNode::createXMLTopNode("msnobj");
        msnObject.addAttribute("Creator", this->Creator.c_str());
        msnObject.addAttribute("Size", toStr(msnobj.Size).c_str());
        msnObject.addAttribute("Type", toStr(msnobj.Type).c_str());
        msnObject.addAttribute("Location", msnobj.Location.c_str());
        msnObject.addAttribute("Friendly", msnobj.Friendly.c_str());
        msnObject.addAttribute("SHA1D", msnobj.SHA1D.c_str());

        if(Type!=11) // voice does not have this field
            msnObject.addAttribute("SHA1C", msnobj.SHA1C.c_str());

        char *xml = msnObject.createXMLString(false);
        msnobj.XMLString = xml;
        free(xml);
        msnObjects.push_front(msnobj);
    }

    void MSNObject::setCreator(std::string creator)
    {
        this->Creator = creator;
    }

    bool MSNObject::getMSNObjectXML(std::string filename, int Type, std::string & msnobj)
    {
        if(msnObjects.empty()) return false;

        std::list<MSNObjectUnit>::iterator i = msnObjects.begin();
        for(; i!=msnObjects.end();i++)
        {
            if((*i).realLocation == filename && 
                    (*i).Type == Type)
            {
                msnobj = (*i).XMLString;
                return true;
            }
        }
        return false;
    }

    bool MSNObject::delMSNObjectByType(int Type)
    {
        bool deleted=false;
        if(msnObjects.empty()) return false;

        std::list<MSNObjectUnit>::iterator i = msnObjects.begin();
        std::list<MSNObjectUnit>::iterator d;
        for(;i!=msnObjects.end();i++)
        {
            if((*i).Type == Type)
            {
                d=i;
                deleted=true;
            }
        }
        if(deleted)
            msnObjects.erase(d);
        return deleted;
    }

    bool MSNObject::getMSNObjectXMLByType(int Type, std::string & xml)
    {
        if(msnObjects.empty()) return false;
        std::list<MSNObjectUnit>::iterator i = msnObjects.begin();
        
        for( ; i!=msnObjects.end();i++)
        {
            if((*i).Type == Type)
            {
                xml = (*i).XMLString;
                return true;
            }
        }
        return false;
    }

    bool MSNObject::getMSNObjectRealPath(std::string xml, std::string & realpath)
    {
        if(msnObjects.empty()) return false;
        XMLNode msnObject = XMLNode::parseString(xml.c_str());
        std::string SHA1D = msnObject.getAttribute("SHA1D",0);

        std::list<MSNObjectUnit>::iterator i = msnObjects.begin();
        
        for( ; i!=msnObjects.end();i++)
        {
            // using SHA1D to ensure if we have this file, even if the name is different
            if((*i).SHA1D == SHA1D)
            {
                realpath = (*i).realLocation;
                return true;
            }
        }
        return false;
    }
}

