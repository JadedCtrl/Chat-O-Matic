/*
 * message.cpp
 * libmsn
 *
 * Created by Mark Rowe on Wed Mar 17 2004.
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

#include <stdlib.h>
#include <message.h>
#include <errorcodes.h>
#include <util.h>
#include <iomanip>
#include <cassert>

namespace MSN 
{
    Message::Message(std::string body_, std::string header_)
        : body(body_), header(header_)
    {
    }
    
    std::string Message::asString() const
    {
        return this->header.asString() + this->body;
    }
    
    std::string Message::operator[](const std::string header_) const
    {
        assert(header_ != "");
        return this->header[header_];
    }
    
    std::string Message::Headers::asString() const
    {
        return this->rawContents;
    }
    
    std::map<std::string, std::string> Message::getFormatInfo() const throw (std::runtime_error)
    {
        std::map<std::string, std::string> formatInfo;
        std::string formatHeader = (*this)["X-MMS-IM-Format"];
        if (formatHeader.empty())
            return formatInfo;
        
        std::vector<std::string> parameters = splitString(formatHeader, ";");
        std::vector<std::string>::iterator i = parameters.begin();
        for (; i != parameters.end(); i++)
        {
	    if (i->at(0) == ' ')
	        i->erase(0, 1);

            std::vector<std::string> pair = splitString(*i, "=");
            if (pair.size() == 2)
                formatInfo[decodeURL(pair[0])] = decodeURL(pair[1]);
            else if (pair.size() == 1)
                formatInfo[decodeURL(pair[0])] = "";
            else
                throw std::runtime_error("Incorrectly specified message format!");
        }
        
        return formatInfo;
    }
    
    void Message::setFormatInfo(std::map<std::string, std::string> & info)
    {
        std::string value;
        std::map<std::string, std::string>::iterator i = info.begin();
        
        if (info.find("FN") != info.end())
        {
            value += "FN=";
            value += encodeURL(info["FN"]);
            value += "; ";
        }
        
        for (; i != info.end(); i++)
        {
            if ((*i).first == "FN")
                continue;
            
            value += encodeURL((*i).first);
            value += "=";
            value += encodeURL((*i).second);
            value += "; ";
        }
        if (value == "")
            return;
        
        assert(value.size() >= 2);
        value = value.substr(0, value.size() - 2);
        this->header.setHeader("X-MMS-IM-Format", value);
    }

    std::string Message::getFontName() const
    {
        return this->getFormatInfo()["FN"];
    }
    
    void Message::setFontName(const std::string & fontName)
    {
        std::map<std::string, std::string> info = this->getFormatInfo();
        info["FN"] = fontName;
        this->setFormatInfo(info);
    }
    
   std::vector<int> Message::getColor() const
    {
        std::string color = this->getFormatInfo()["CO"];
        assert(color.size() <= 6 && color.size() >= 0);
        color.insert(0U, 6 - color.size(), '0');
        int r = 0, g = 0, b = 0;
        
        b = strtol(color.substr(0, 2).c_str(), NULL, 16);
        g = strtol(color.substr(2, 2).c_str(), NULL, 16);        
        r = strtol(color.substr(4, 2).c_str(), NULL, 16);
        
        std::vector<int> out;
        out.push_back(r);
        out.push_back(g);
        out.push_back(b);
        return out;
    }
    
    std::string Message::getColorAsHTMLString() const
    {
        std::vector<int> color = this->getColor();
        std::ostringstream s;
        s << std::hex << std::setfill('0') << std::setw(2) << color[0];
        s << std::hex << std::setfill('0') << std::setw(2) << color[1];
        s << std::hex << std::setfill('0') << std::setw(2) << color[2];
        
        assert(s.str().size() == 6);
        return s.str();
    }
    
    void Message::setColor(std::vector<int> color)
    {
        std::map<std::string, std::string> info = this->getFormatInfo();
        assert(color.size() == 3);
        
        std::ostringstream s;
        s << std::hex << std::setfill('0') << std::setw(2) << color[2];
        s << std::hex << std::setfill('0') << std::setw(2) << color[1];
        s << std::hex << std::setfill('0') << std::setw(2) << color[0];
        
        assert(s.str().size() == 6);
        info["CO"] = s.str();
        this->setFormatInfo(info);
    }
    
    void Message::setColor(std::string color)
    {
        color.insert(0U, 6 - color.size(), '0');
        int r = 0, g = 0, b = 0;
        
        r = strtol(color.substr(0, 2).c_str(), NULL, 16);
        g = strtol(color.substr(2, 2).c_str(), NULL, 16);        
        b = strtol(color.substr(4, 2).c_str(), NULL, 16);
        
        std::vector<int> v;
        v.push_back(r);
        v.push_back(g);
        v.push_back(b);
        this->setColor(v);
    }
    
    void Message::setColor(int red, int green, int blue)
    {
        std::vector<int> v;
        v.push_back(red);
        v.push_back(green);
        v.push_back(blue);
        this->setColor(v);        
    }

    
    int Message::getFontEffects() const
    {
        int retVal = 0;
        std::string fontEffects = this->getFormatInfo()["EF"];
        
        if (fontEffects.find("B") != std::string::npos)
            retVal |= BOLD_FONT;
        
        if (fontEffects.find("I") != std::string::npos)
            retVal |= ITALIC_FONT;
        
        if (fontEffects.find("U") != std::string::npos)
            retVal |= UNDERLINE_FONT;
        
        if (fontEffects.find("S") != std::string::npos)
            retVal |= STRIKETHROUGH_FONT;
        
        return retVal;
    }
    
    void Message::setFontEffects(int fontEffects)
    {
        std::string effects;
        std::map<std::string, std::string> info = this->getFormatInfo();
        
        if (fontEffects & BOLD_FONT)
            effects += "B";
        
        if (fontEffects & ITALIC_FONT)
            effects += "I";
        
        if (fontEffects & UNDERLINE_FONT)
            effects += "U";
        
        if (fontEffects & STRIKETHROUGH_FONT)
            effects += "S";
        
        info["EF"] = effects;
        this->setFormatInfo(info);
    }
    
    Message::CharacterSet Message::getFontCharacterSet() const
    {
        std::string fontCharacterSet = this->getFormatInfo()["CS"];
        int c = strtol(fontCharacterSet.c_str(), NULL, 16);
        return (Message::CharacterSet) c;
    }
    
    void Message::setFontCharacterSet(CharacterSet cs)
    {
        std::map<std::string, std::string> info = this->getFormatInfo();
        std::ostringstream s;
        
        s << std::hex << (int) cs;
        info["CS"] = s.str();
        
        this->setFormatInfo(info);
    }
    
    Message::FontFamily Message::getFontFamily() const
    {
        std::string fontFamily = this->getFormatInfo()["PF"];
        if (fontFamily.size() < 1)
            return (Message::FontFamily) 0;
        int family = decimalFromString(fontFamily.substr(0, 1));
        return (Message::FontFamily) family;
    }
    
    Message::FontPitch Message::getFontPitch() const
    {
        std::string fontPitch = this->getFormatInfo()["PF"];
        if (fontPitch.size() < 2)
            return (Message::FontPitch) 0;
        int pitch = decimalFromString(fontPitch.substr(1, 1));
        return (Message::FontPitch) pitch;        
    }
    
    void Message::setFontFamilyAndPitch(Message::FontFamily fontFamily, Message::FontPitch fontPitch)
    {
        std::map<std::string, std::string> info = this->getFormatInfo();
        std::ostringstream s;
        
        s << fontFamily << fontPitch;
        info["PF"] = s.str();
        
        this->setFormatInfo(info);
    }
    
    bool Message::isRightAligned() const
    {
        return this->getFormatInfo()["RL"] == "1";
    }
    
    void Message::Headers::setHeader(const std::string header, const std::string value)
    {
        if ((*this)[header] == "")
        {
            assert(this->rawContents.size() >= 2);
            this->rawContents.insert(this->rawContents.size() - 2, header + ": " + value + "\r\n");
        }
        else
        {
            size_t position = this->rawContents.find(header + ": ");
            assert(position != std::string::npos);
            
            size_t eol = this->rawContents.find("\r\n", position);
            if (eol == std::string::npos)
                eol = this->rawContents.size();
            
            this->rawContents.erase(position, eol - position + 2);
            this->rawContents.insert(position, header + ": " + value + "\r\n");
        }
    }
    
    std::string Message::Headers::operator[](const std::string header_) const
    {
        std::string retval;
        std::string::iterator i;
            
        if (this->rawContents.substr(0U, header_.size()) == header_)
        {
            retval = this->rawContents;
        } else {
            std::string tmp = "\r\n" + header_;
            size_t position = this->rawContents.find(tmp);
            if (position == std::string::npos)
                return "";
                
            retval = this->rawContents.substr(position + 2);
        }
        
        retval = retval.substr(retval.find(':') + 1);
        while (isspace(retval[0]))
            retval.erase(retval.begin());
        
        for (i = retval.begin(); i != retval.end(); i++)
        {
            if (*i == '\r')
            {
                return retval.substr(0, std::distance(retval.begin(), i));
            }
        }
        return "";
    }
}
