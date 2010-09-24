#ifndef __msn_util_h__
#define __msn_util_h__

/*
 * util.h
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
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/hmac.h>
#include <openssl/des.h>

#include <string>
#include <sstream>
#include <map>
#include <vector>
#include <stdexcept>
#include <string>

#ifdef _MSC_VER
#pragma warning( disable : 4290 )
#endif

// this is for CHL command
#define szClientID "PROD0114ES4Z%Q5W"
#define szClientCode "PK}_A_0N_K%O?A9S"

#ifndef U8
#define U8 unsigned char
#endif
#ifndef U16
#define U16 unsigned short
#endif
#ifndef U32
#define U32 unsigned int
#endif
#define FB_UNI 0xFFFd

// for libsiren
#define RIFF_ID 0x46464952
#define WAVE_ID 0x45564157
#define FMT_ID 0x20746d66
#define DATA_ID 0x61746164
#define FACT_ID 0x74636166

typedef struct
{
    unsigned int chunk_id;
    unsigned int chunk_size;
} wav_data;

typedef struct
{
    unsigned int chunk_id;
    unsigned int chunk_size;
    unsigned int type_id;
} riff_data;

typedef struct
{
    unsigned short format;
    unsigned short channels;
    unsigned int sample_rate;
    unsigned int byte_rate;
    unsigned short block_align;
    unsigned short bits_per_sample;
} fmt_chunk;

typedef struct
{
    fmt_chunk fmt;
    unsigned short extra_size;
    unsigned char *extra_content;
} fmt_chunk_ex;

#define IDX(val, i) ((unsigned int) ((unsigned char *) &val)[i])

#define GUINT16_FROM_LE(val) ((unsigned short) (IDX (val, 0) + (unsigned short) IDX (val, 1) * 256))
#define GUINT32_FROM_LE(val) ((unsigned int) (IDX (val, 0) + IDX (val, 1) * 256 + \
                                              IDX (val, 2) * 65536 + IDX (val, 3) * 16777216))

namespace MSN 
{
    /** URL-encode a string
     *
     * @param  s  The string to encode.
     * @return    A string with all non-alphanumeric characters replaced by their
     *            URL-encoded equivalent.
     */
    std::string encodeURL(const std::string & s);
    
    /** URL-decode a string
     *
     * @param  s  The URL-encoded string to decode.
     * @return    A string with all URL-encoded sequences replaced by their
     *            @c ASCII equivalent.
     */
    std::string decodeURL(const std::string & s);
    
    /** Split a string containing a hostname and port number into its respective parts.
     *
     * @param  address       A string in the form "hostname:port".
     * @param  default_port  A port number to return in the event that ":port" is omitted from @a address.
     * @return               A pair containing the hostname and port number.
     */
    std::pair<std::string, int> splitServerAddress(const std::string & address, int default_port=1863);
    
    /** Compare two strings in a case insensitive fashion
     */
    int nocase_cmp(const std::string & s1, const std::string & s2);
    
    /** Split @a string at each occurence of @a separator.
     */
    std::vector<std::string> splitString(const std::string & string, const std::string & separator, bool suppressBlanks=true);
    
    std::string toStr(int var);
    std::string unsignedToStr(unsigned int var);
    /** Convert a string, @a s, that contains decimal digits into an unsigned int.
     */
    unsigned int decimalFromString(const std::string & s) throw (std::logic_error);

    U32 _ucs2_utf8(U8 *dst, U8 *src, U32 nchar);
    U32 _utf8_ucs2(U8 *dst, U8 *src);

    /** represents a contact pesonal message */
    struct personalInfo
    {
        std::string PSM; /**< personal status message */
        std::string mediaApp; /**< iTunes, Winamp or keep it empty */
        std::string mediaType; /**<  'Music', 'Games' or 'Office' */
        bool mediaIsEnabled; /**<  enable/disable the Current Media setting */
        std::string mediaFormat; /**< for example, "{0} - {1}" */
        std::vector<std::string> mediaLines; /**<  index 0 will be {0}, etc.. */

        personalInfo() {
            mediaIsEnabled = false;
        }
    };

    struct hotmailInfo
    {
        std::string rru;
        std::string url;
        std::string id;
        std::string sl;
        std::string kv;
        std::string sid;
        std::string MSPAuth;
        std::string creds;
    };

    /** Represents the lists present on server side */
    typedef enum
    {
        LST_AB = 1,        /**< Address book */
        LST_AL = 2,        /**< Allow */
        LST_BL = 4,        /**< Block */
        LST_RL = 8,        /**< Reverse */
        LST_PL = 16        /**< Pending */
    }ContactList;
 
    struct tagMSGRUSRKEY
    {
         unsigned int uStructHeaderSize; // 28. Does not count data
         unsigned int uCryptMode; // CRYPT_MODE_CBC (1)
         unsigned int uCipherType; // TripleDES (0x6603)
         unsigned int uHashType; // SHA1 (0x8004)
         unsigned int uIVLen;    // 8
         unsigned int uHashLen;  // 20
         unsigned int uCipherLen; // 72
         // Data
         unsigned char aIVBytes[8];
         unsigned char aHashBytes[20];
         unsigned char aCipherBytes[72];
    };

    /** represents an offline message */
    typedef struct 
    {
         std::string from; /**< sender passport */
         std::string fromFN; /**< sender nickname */
         std::string id; /**< ID of this offline message */
    } eachOIM;

    std::string new_branch();
    std::string generate_soap_auth(std::string user, std::string pass, std::string ticket);
    std::string mdi_encrypt(std::string key, std::string nonce);
    std::string b64_decode(const char *input);
    std::string b64_encode(const char *input, int size);

    unsigned int little2big_endian(unsigned int i);
    int FileSize(const char* sFileName);
    void DoMSNP11Challenge(const char *szChallenge, char *szOutput);

    // stolen from kopete
    /** List of possible capabilities for a contact */
    typedef enum
    {
        WindowsMobile = 0x1,
        InkGifSupport = 0x4,
        InkIsfSupport = 0x8,
        SupportWebcam = 0x10,
        SupportMultiPacketMessaging = 0x20,
        MSNMobileDevice = 0x40,
        MSNDirectDevice = 0x80,
        WebMessenger = 0x100,
        OtherSideWebMessenger = 0x200,
        InternalMicrosoftClient = 0x800, //Internal Microsoft client and/or Microsoft Office Live client.
        MSNSpace = 0x1000,
        WinXPMediaCenter = 0x2000, // This means you are using Windows XP Media Center Edition.
        SupportDirectIM =  0x4000,
        SupportWinks = 0x8000,
        MSNSearch = 0x10000,
        VoiceClips = 0x40000,
        SecureChannel = 0x80000,
        SIPInvitations = 0x100000,
        SharingFolders = 0x400000,
        MSNC1 = 0x10000000,
        MSNC2 = 0x20000000,
        MSNC3 = 0x30000000,
        MSNC4 = 0x40000000,
        MSNC5 = 0x50000000,
        MSNC6 = 0x60000000,
        MSNC7 = 0x70000000
    } MSNClientInformationFields;

    /** Defines the file transfer type */
    enum fileTransferType
    {
        FILE_TRANSFER_WITH_PREVIEW = 0x0, /**< With preview */
        FILE_TRANSFER_WITHOUT_PREVIEW = 0x1, /**< Without preview */
        FILE_TRANSFER_BACKGROUND_SHARING = 0x4, /**< Transfer of a sharing background */
        // it is not a simple jpg file, there is a cab file inside it
        FILE_TRANSFER_BACKGROUND_SHARING_CUSTOM = 0xC /**< Custom and not supported by libmsn yet */ 
    };

    /** Type of the error when a file transfer fails */
    enum fileTransferError
    {
        FILE_TRANSFER_ERROR_USER_CANCELED, /**< The other user canceled */
        FILE_TRANSFER_ERROR_UNKNOWN /**< Unknown error */
    };

    /** Represents a file transfer request */
    typedef struct 
    {
        int type;                 /**< 0 = no preview, 1 = has preview, 4 = background sharing */
        unsigned int sessionId;   /**< Id of this session */
        std::string userPassport; /**< passport of the origin or the destination */
        std::string filename;     /**< name the file to receive, or the path of the file to send */
        std::string friendlyname; /**< suggested name <- required when sending a file */
        std::string preview;      /**< base64 encoded 96x96 png file, if applicable */
        unsigned long long filesize; /**< size of the file to send or receive */
    } fileTransferInvite;
    
    void libmsn_Siren7_DecodeVoiceClip(std::string input_file);
    void libmsn_Siren7_EncodeVoiceClip(std::string input_file);
}
#endif
