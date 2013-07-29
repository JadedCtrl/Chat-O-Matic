#ifndef __msn_message_h__
#define __msn_message_h__

/*
 * message.h
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

#ifdef WIN32
#include <windows.h>
#endif
#include <string>
#include <map>
#include <vector>
#include <stdexcept>

#include "libmsn_export.h"

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

namespace MSN 
{
    
    /** This class represents an MSN message
     *
     *  It may or may not represent an @e instant @e message.
     *
     *  @todo  Complete read/write support for formatting messages.
     */
    class LIBMSN_EXPORT Message
    {
public:
        enum FontEffects
        {
            BOLD_FONT = 1,
            ITALIC_FONT = 2,
            UNDERLINE_FONT = 4,
            STRIKETHROUGH_FONT = 8
        };
        
#ifdef WIN32
  typedef int CharacterSet;
  typedef int FontFamily;
  typedef int FontPitch;
#else        
        enum CharacterSet
        {
            ANSI_CHARSET = 0x00,
            DEFAULT_CHARSET = 0x01,
            SYMBOL_CHARSET = 0x02,
            MAC_CHARSET = 0x4d,
            SHIFTJIS_CHARSET = 0x80,
            HANGEUL_CHARSET = 0x81,
            JOHAB_CHARSET = 0x82,
            GB2312_CHARSET = 0x86,
            CHINESEBIG5_CHARSET = 0x88,
            GREEK_CHARSET = 0xa1,
            TURKISH_CHARSET = 0xa2,
            VIETNAMESE_CHARSET = 0xa3,
            HEBREW_CHARSET = 0xb1,
            ARABIC_CHARSET = 0xb2,
            BALTIC_CHARSET = 0xba,
            RUSSIAN_CHARSET_DEFAULT = 0xcc,
            THAI_CHARSET = 0xde,
            EASTEUROPE_CHARSET = 0xee,
            OEM_DEFAULT = 0xff
        };
        
        enum FontFamily
        {
            FF_DONTCARE = 0,
            FF_ROMAN = 1,
            FF_SWISS = 2,
            FF_MODERN = 3,
            FF_SCRIPT = 4,
            FF_DECORATIVE = 5
        };
        
        enum FontPitch
        {
            DEFAULT_PITCH = 0,
            FIXED_PITCH = 1,
            VARIABLE_PITCH = 2
        };
#endif
        
        class Headers
        {
public:
            Headers(const std::string & rawContents_) : rawContents(rawContents_) {};
            Headers() : rawContents("") {};
            std::string asString() const;
            std::string operator[](const std::string header) const;
            void setHeader(const std::string header, const std::string value);

private:
            std::string rawContents;
        };

private:
        std::string body;
        Message::Headers header;

public:
        /** Create a message with the specified @a body and @a mimeHeader.
         */
        Message(std::string body, std::string mimeHeader="MIME-Version: 1.0\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n");
        
        /** Convert the Message into a string.
         *
         *  This returns a string containing the MIME headers separated from the
         *  message body by a blank line.
         */
        std::string asString() const;
        
        /** Return the value of the MIME header named @a header.
         *
         *  @return  The value of the MIME header if present, or "" if not found.
         */ 
        std::string operator[](const std::string header) const;
        void setHeader(const std::string name, const std::string value) { header.setHeader(name, value); };
        
        /** Return the body portion of this Message.
         */
        std::string getBody() const { return body; };
        
        /** Return the font name used in this Message.
         *
         *  @return  The font name used for this Message, or "" if none specified.
         */
        std::string getFontName() const;
        
        /** Set font name for use in this Message.
         */
        void setFontName(const std::string & fontName);
        
        /** Get the color used in this Message.
         */
        std::vector<int> getColor() const;
        std::string getColorAsHTMLString() const;
        
        /** Set the color used in this Message.
         */
        void setColor(std::vector<int> color);
        void setColor(std::string color);
        void setColor(int red, int green, int blue);
        
        /** Return the font effects used in this Message.
         *
         *  @return An integer that is a bitwise-or of Message::FontEffects members.
         */
        int getFontEffects() const;
        
        /** Set the font effects for use in this Message.
         *
         *  @param fontEffects Bitwise-or of Message::FontEffects members.
         */
        void setFontEffects(int fontEffects);
        
        /** Return the character set that the font uses in this Message.
         */
        CharacterSet getFontCharacterSet() const;
        
        /** Set the character set that the font should use for this Message.
         */
        void setFontCharacterSet(CharacterSet cs);
        
        /** Return the font family used in this Message.
         */
        FontFamily getFontFamily() const;
        
        /** Return the font pitch used in this Message.
         */
        FontPitch getFontPitch() const;
        
        /** Set the font family and pitch to be used for this Message.
         */
        void setFontFamilyAndPitch(Message::FontFamily fontFamily, Message::FontPitch fontPitch);
        
        /** Is the Message to be right-aligned?
         */
        bool isRightAligned() const;

private:
        std::map<std::string, std::string> getFormatInfo() const throw (std::runtime_error);
        void setFormatInfo(std::map<std::string, std::string> & info);
    };
    
}
#endif
