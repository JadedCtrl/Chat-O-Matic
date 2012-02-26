/*
 * util.cpp
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
#include <iostream>
#include <util.h>
#include <unistd.h>
#include <sstream>
#include <errno.h>
#include <cctype>
#include <fstream>
#include <openssl/rand.h>
#include <cstring>
#include <sys/timeb.h>
#include <md5.h>
#include <libsiren/siren7.h>

#ifdef _WIN32
#define random rand
#endif

namespace MSN 
{    
    std::pair<std::string, int> splitServerAddress(const std::string & address, int default_port)
    {
        size_t pos;
        std::string host = address;
        int port = default_port;
        
        if ((pos = address.find(":")) != std::string::npos)
        {
            std::string port_s = address.substr(pos + 1);
            host = address.substr(0, pos);
            port = decimalFromString(port_s);
        }

        if (host == "" || port < 0)
            throw std::runtime_error("Invalid zero-length address or negative port number!");

        return std::make_pair(host, port);
    }

    std::string decodeURL(const std::string & s)
    {
        std::string out;
        std::string::const_iterator i;
        
        for (i = s.begin(); i != s.end(); i++)
        {
            if (*i == '%')
            {
                char entity[3] = {0, 0, 0};
                if (++i == s.end())
                    break;
                
                entity[0] = *i;

                bool doBreak = false;
                if (++i != s.end())
                    entity[1] = *i;
                else
                    doBreak = true;
                    
                int c = strtol(entity, NULL, 16);
                out += c;

                if (doBreak)
                    break;
            }
            else
                out += *i;
        }
        return out;
    }
    
    std::string encodeURL(const std::string & s)
    {
        std::string out;
        std::string::const_iterator i;
        
        for (i = s.begin(); i != s.end(); i++)
        {
            if(!(isalpha(*i) || isdigit(*i)))
            {
                unsigned char high_nibble = ((unsigned char) *i) >> 4;
                unsigned char low_nibble = ((unsigned char) *i) & 0x0F;
                out += '%';
                out += (high_nibble < 0x0A ? '0' + high_nibble : 'A' + high_nibble - 0x0A);
                out += (low_nibble < 0x0A ? '0' + low_nibble : 'A' + low_nibble - 0x0A);
                
                continue;
            }
            out += *i;
        }

        return out;
    }
    
    std::vector<std::string> splitString(const std::string & s, const std::string & sep, bool suppressBlanks)
    {
        std::vector<std::string> array;     
        size_t position, last_position;
        
        last_position = position = 0;
        while (position + sep.size() <= s.size())
        {
            if (s[position] == sep[0] && s.substr(position, sep.size()) == sep)
            {
                if (!suppressBlanks || position - last_position > 0)
                    array.push_back(s.substr(last_position, position - last_position));
                last_position = position = position + sep.size();
            }
            else
                position++;
        }
        if (!suppressBlanks || last_position - s.size())
            array.push_back(s.substr(last_position));
        
        return array;
    }
    
    int nocase_cmp(const std::string & s1, const std::string& s2) 
    {
        std::string::const_iterator it1, it2;
        
        for (it1 = s1.begin(), it2 = s2.begin();
             it1 != s1.end() && it2 != s2.end();
             ++it1, ++it2)
        { 
            if (std::toupper(*it1) != std::toupper(*it2)) 
                return std::toupper(*it1) - std::toupper(*it2);
        }
        size_t size1 = s1.size(), size2 = s2.size(); 
        return (int) (size1 - size2);
    }

    std::string toStr(int var) {
        std::ostringstream tmp; tmp << var; return tmp.str();
    }

    std::string unsignedToStr(unsigned int var) {
        std::ostringstream tmp; tmp << var; return tmp.str();
    } 

    unsigned int decimalFromString(const std::string & s) throw (std::logic_error)
    {
        unsigned int result = strtol(s.c_str(), NULL, 10);
        errno = 0;
        if (result == 0 && errno != 0)
            throw std::logic_error(strerror(errno));
        return result;
    }

    std::string hmac_sha(std::string key, std::string message)
    {
        unsigned int buf_len=0;
        unsigned char buf[50];
        memset(&buf,0,50);
        HMAC(EVP_sha1(), key.c_str(), key.length(), (const unsigned char*)message.c_str(), message.length(), buf, &buf_len);
        std::string a((char *)buf,buf_len);
        return a;
    }

    std::string derive_key(std::string key, std::string magic)
    {
       std::string hash1(hmac_sha(key, magic));
       std::string hash2(hmac_sha(key, hash1+magic));
       std::string hash3(hmac_sha(key, hash1));
       std::string hash4(hmac_sha(key, hash3+magic));
       std::string final(hash2+hash4.substr(0,4));
       return final;
    }
   
    std::string b64_encode(const char *input, int t)
    {
        BIO *mbio, *b64bio, *bio;
        char *outbuf;
        int inlen, outlen;
        char *output;

        mbio = BIO_new(BIO_s_mem());
        b64bio = BIO_new(BIO_f_base64());
        BIO_set_flags(b64bio, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_push(b64bio, mbio);

        inlen = t ;
        if (BIO_write(bio, input, inlen) != inlen) {
            return "";
        }
        BIO_flush(bio);

        outlen = BIO_get_mem_data(bio, &outbuf);

        output = (char*) malloc(outlen+1);
        memcpy(output, outbuf, outlen);
        output[outlen] = '\0';
        std::string output1(output);
        BIO_free_all(bio);
        free(output);

        return output1;
    }
   
    std::string b64_decode(const char *input)
    {
        BIO *mbio, *b64bio, *bio;
        int inlen, outlen;
        char *output;

        mbio = BIO_new_mem_buf((void *)input, -1);
        b64bio = BIO_new(BIO_f_base64());
        BIO_set_flags(b64bio, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_push(b64bio, mbio);

        inlen = strlen(input);
        outlen = inlen*2;

        output = (char*) malloc(outlen+1);

        if ((outlen = BIO_read(bio, output, outlen)) <= 0) {
            return "";
        }
        output[outlen] = '\0';
        std::string output1(output, outlen);
        free(output);
        BIO_free_all(bio);

        return output1;
    }
   
    std::string mdi_encrypt(std::string key, std::string nonce)
    {
        tagMSGRUSRKEY MSGUSRKEY;
        std::string key1,key2,key3;

        key1 = b64_decode(key.c_str());
        key2 = derive_key(key1, "WS-SecureConversationSESSION KEY HASH");
        key3 = derive_key(key1, "WS-SecureConversationSESSION KEY ENCRYPTION");

        std::string hash = hmac_sha(key2, nonce);

        unsigned char workvec[8];
        RAND_bytes(workvec, 8);
        des_key_schedule ks1,ks2,ks3;

        const char *one=key3.c_str();
        const char *two=key3.c_str()+8;
        const char *three=key3.c_str()+16;

        des_set_key((C_Block *)one,ks1);
        des_set_key((C_Block *)two,ks2);
        des_set_key((C_Block *)three,ks3);

        unsigned char output[72];
        memset(&output,0,72);

        memcpy(&MSGUSRKEY.aIVBytes, &workvec, sizeof(workvec));
        memcpy(&MSGUSRKEY.aHashBytes, hash.c_str() , hash.length());

        // ugly, but I think it is working properly
        std::ostringstream buf_;
        buf_ << nonce << "\x08\x08\x08\x08\x08\x08\x08\x08";
        DES_ede3_cbc_encrypt((const unsigned char*)buf_.str().c_str(),output,buf_.str().size(),&ks1,&ks2,&ks3,(C_Block *)workvec,DES_ENCRYPT);

        MSGUSRKEY.uStructHeaderSize=28;
        MSGUSRKEY.uCryptMode=1;
        MSGUSRKEY.uCipherType=0x6603;
        MSGUSRKEY.uHashType=0x8004;
        MSGUSRKEY.uIVLen=8;
        MSGUSRKEY.uHashLen=hash.length();
        MSGUSRKEY.uCipherLen=72;

        // set 
        memcpy(&MSGUSRKEY.aCipherBytes,output, 72);
        unsigned char a[129]; // last is \0 to b64_encode
        memset(&a,0,129);
        memcpy(&a, &MSGUSRKEY, sizeof(tagMSGRUSRKEY));

        return b64_encode((const char*)a,128);
    }

   void DoMSNP11Challenge(const char *szChallenge, char *szOutput) 
   {
        int i;
        md5_state_t state;
        md5_byte_t digest[16];
        md5_init(&state);
        md5_append(&state, (const md5_byte_t *)szChallenge, strlen(szChallenge));
        md5_append(&state, (const md5_byte_t *)szClientCode, strlen(szClientCode));
        md5_finish(&state, digest);

        unsigned char pMD5Hash[16];
        memcpy(pMD5Hash,digest,16);
        int *pMD5Parts=(int *)digest;
        for (i=0; i<4; i++) {
            pMD5Parts[i]&=0x7FFFFFFF;
        }
        int nchlLen=strlen(szChallenge)+strlen(szClientID);
        if (nchlLen%8!=0)
            nchlLen+=8-(nchlLen%8);
        char *chlString=new char[nchlLen];
        memset(chlString,'0',nchlLen);
        memcpy(chlString,szChallenge,strlen(szChallenge));
        memcpy(chlString+strlen(szChallenge),szClientID,strlen(szClientID));
        int *pchlStringParts=(int *)chlString;

        long long nHigh=0;
        long long nLow=0;

        for (i=0; i<(nchlLen/4)-1; i+=2) {
            long long temp=pchlStringParts[i];
            temp=(pMD5Parts[0] * (((0x0E79A9C1 * (long long)pchlStringParts[i]) % 0x7FFFFFFF)+nHigh) + pMD5Parts[1])%0x7FFFFFFF;
            nHigh=(pMD5Parts[2] * (((long long)pchlStringParts[i+1]+temp) % 0x7FFFFFFF) + pMD5Parts[3]) % 0x7FFFFFFF;
            nLow=nLow + nHigh + temp;
        }
        nHigh=(nHigh+pMD5Parts[1]) % 0x7FFFFFFF;
        nLow=(nLow+pMD5Parts[3]) % 0x7FFFFFFF;
        delete[] chlString;

        unsigned int *pNewHash=(unsigned int *)pMD5Hash;

        pNewHash[0]^=nHigh;
        pNewHash[1]^=nLow;
        pNewHash[2]^=nHigh;
        pNewHash[3]^=nLow;

        char szHexChars[]="0123456789abcdef";
        for (i=0; i<16; i++) {
            szOutput[i*2]=szHexChars[(pMD5Hash[i]>>4)&0xF];
            szOutput[(i*2)+1]=szHexChars[pMD5Hash[i]&0xF];
        }
    }
    // 4-byte number
    unsigned int little2big_endian(unsigned int i)
    {
        return((i&0xff)<<24)+((i&0xff00)<<8)+((i&0xff0000)>>8)+((i>>24)&0xff);
    }

    int FileSize(const char* sFileName)
    {
        std::ifstream f;
        f.open(sFileName, std::ios_base::binary | std::ios_base::in);
        if (!f.good() || f.eof() || !f.is_open()) { return 0; }
        f.seekg(0, std::ios_base::beg);
        std::ifstream::pos_type begin_pos = f.tellg();
        f.seekg(0, std::ios_base::end);
        return static_cast<int>(f.tellg() - begin_pos);
    }

    std::string new_branch()
    {
        struct timeb t;
        ftime(&t);
        char branch[100];
        srand(t.millitm);
        unsigned int a=random();
        srand(a);
        unsigned short b=random();
        srand(b);
        unsigned short c=random();
        srand(c);
        unsigned short d=random();
        srand(d);
        double e=random();
        sprintf(branch,"{%.8X-%.4X-%.4X-%.4X-%.12X}",a,b,c,d,(unsigned int)e);
        std::string newbranch(branch);
        return newbranch;
    }

    // Code from http://search.cpan.org/src/DANKOGAI/Jcode-2.06/Unicode/uni.c
    U32 _ucs2_utf8(U8 *dst, U8 *src, U32 nchar)
    {
        U32 ucs2;
        U32 result = 0;
        for (nchar /= 2; nchar > 0; nchar--, src += 2) {
            ucs2 = src[0]*256 + src[1];
            if (ucs2 < 0x80){      /* 1 byte */
                *dst++ = ucs2;
                result += 1;
            }else if (ucs2 < 0x800){ /* 2 bytes */
                *dst++ = (0xC0 | (ucs2 >> 6));
                *dst++ = (0x80 | (ucs2 & 0x3F));
                result += 2;
            }else{                /*  3 bytes */
                *dst++ = (0xE0 | (ucs2 >> 12));
                *dst++ = (0x80 | ((ucs2 >> 6) & 0x3F));
                *dst++ = (0x80 | (ucs2 & 0x3F));
                result += 3;
            }
        }
        *dst  = '\0';
        return result;
    }

    U32 _utf8_ucs2(U8 *dst, U8 *src)
    {
        U32  ucs2;
        U8 c1, c2, c3;
        U32 result = 0;
      
        for(; *src != '\0'; src++, result++){
            if (*src < 0x80) {     /* 1 byte */
                ucs2 = *src;
            }else if (*src < 0xE0){ /* 2 bytes */
                if (src[1]){
                    c1 = *src++; c2 = *src;
                    ucs2 = ((c1 & 0x1F) << 6) | (c2 & 0x3F);
                }else{
                    ucs2 = FB_UNI;
                }
            }else{                 /* 3 bytes */
                if (src[1] && src[2]){
                    c1 = *src++; c2 = *src++; c3 = *src;
                    ucs2 = ((c1 & 0x0F) << 12) | ((c2 & 0x3F) << 6)| (c3 & 0x3F);
                }else{
                    ucs2 = FB_UNI;
                if (src[1])
                    src++;
                }
            }
            *dst++ = (ucs2 & 0xff00) >> 8; /* 1st byte */
            *dst++ = (ucs2 & 0xff);        /* 2nd byte */;
        }
        return result * 2;
    }
    
    // convert from siren codec to a regular wav file
    void
    libmsn_Siren7_DecodeVoiceClip(std::string input_file)
    {
        FILE * input;
        FILE * output;
        riff_data riff_header;
        wav_data current_chunk;
        fmt_chunk_ex fmt_info;
        unsigned char *out_data = NULL;
        unsigned char *out_ptr = NULL;
        unsigned char in_buffer[40];
        unsigned int file_offset;
        unsigned int chunk_offset;

        std::string new_voice(input_file.c_str());
        std::string old_voice = new_voice + "-old";
        rename(new_voice.c_str(), old_voice.c_str());

        SirenDecoder decoder = Siren7_NewDecoder (16000);

        input = fopen (old_voice.c_str(), "rb");
        output = fopen (new_voice.c_str(), "wb");

        file_offset = 0;
        fread (&riff_header, sizeof (riff_data), 1, input);
        file_offset += sizeof (riff_data);

        riff_header.chunk_id = GUINT32_FROM_LE (riff_header.chunk_id);
        riff_header.chunk_size = GUINT32_FROM_LE (riff_header.chunk_size);
        riff_header.type_id = GUINT32_FROM_LE (riff_header.type_id);

        if (riff_header.chunk_id == RIFF_ID && riff_header.type_id == WAVE_ID)
        {
            while (file_offset < riff_header.chunk_size)
            {
                fread (&current_chunk, sizeof (wav_data), 1, input);
                file_offset += sizeof (wav_data);
                current_chunk.chunk_id = GUINT32_FROM_LE (current_chunk.chunk_id);
                current_chunk.chunk_size = GUINT32_FROM_LE (current_chunk.chunk_size);

                chunk_offset = 0;
                if (current_chunk.chunk_id == FMT_ID)
                {
                    fread (&fmt_info, sizeof (fmt_chunk), 1, input);
                    /* Should convert from LE the fmt_info structure, but it's not necessary... */
                    if (current_chunk.chunk_size > sizeof (fmt_chunk))
                    {
                        fread (&(fmt_info.extra_size), sizeof (short), 1, input);
                        fmt_info.extra_size = GUINT32_FROM_LE (fmt_info.extra_size);
                        fmt_info.extra_content = (unsigned char *) malloc (fmt_info.extra_size);
                        fread (fmt_info.extra_content, fmt_info.extra_size, 1, input);
                    }
                    else
                    {
                        fmt_info.extra_size = 0;
                        fmt_info.extra_content = NULL;
                    }
                }
                else if (current_chunk.chunk_id  == DATA_ID)
                {
                    out_data = (unsigned char *) malloc (current_chunk.chunk_size * 16);
                    out_ptr = out_data;
                    while (chunk_offset + 40 <= current_chunk.chunk_size)
                    {
                        fread (in_buffer, 1, 40, input);
                        Siren7_DecodeFrame (decoder, in_buffer, out_ptr);
                        out_ptr += 640;
                        chunk_offset += 40;
                    }
                    fread (in_buffer, 1, current_chunk.chunk_size - chunk_offset, input);
                }
                else
                {
                    fseek (input, current_chunk.chunk_size, SEEK_CUR);
                }

                file_offset += current_chunk.chunk_size;
            }
        }

        /* The WAV heder should be converted TO LE, but should be done inside the library and it's not important for now ... */
        fwrite (&(decoder->WavHeader), sizeof (decoder->WavHeader), 1, output);
        fwrite (out_data, 1, GUINT32_FROM_LE (decoder->WavHeader.DataSize), output);
        fclose (output);

        Siren7_CloseDecoder (decoder);

        free (out_data);
        free (fmt_info.extra_content);

        // remove the siren encoded file
        unlink(old_voice.c_str());
    }

    // convert to siren codec from a regular wav file
    void
    libmsn_Siren7_EncodeVoiceClip(std::string input_file)
    {
        FILE * input;
        FILE * output;
        riff_data riff_header;
        wav_data current_chunk;
        fmt_chunk_ex fmt_info;
        unsigned char *out_data = NULL;
        unsigned char *out_ptr = NULL;
        unsigned char InBuffer[640];
        unsigned int fileOffset;
        unsigned int chunkOffset;
     
        SirenEncoder encoder = Siren7_NewEncoder(16000);
 
        std::string new_voice(input_file.c_str());
        std::string old_voice = new_voice + "-old";
        rename(new_voice.c_str(), old_voice.c_str());
    
        input = fopen (old_voice.c_str(), "rb");
        output = fopen (new_voice.c_str(), "wb");
     
        fileOffset = 0;
        fread(&riff_header, sizeof(riff_data), 1, input);
        fileOffset += sizeof(riff_data);
     
        riff_header.chunk_id = GUINT32_FROM_LE(riff_header.chunk_id);
        riff_header.chunk_size = GUINT32_FROM_LE(riff_header.chunk_size);
        riff_header.type_id = GUINT32_FROM_LE(riff_header.type_id);
     
        if (riff_header.chunk_id == RIFF_ID && riff_header.type_id == WAVE_ID) {
              while (fileOffset < riff_header.chunk_size) {
                    fread(&current_chunk, sizeof(wav_data), 1, input);
                    fileOffset += sizeof(wav_data);
                    current_chunk.chunk_id = GUINT32_FROM_LE(current_chunk.chunk_id);
                    current_chunk.chunk_size = GUINT32_FROM_LE(current_chunk.chunk_size);
     
                    chunkOffset = 0;
                    if (current_chunk.chunk_id == FMT__ID) {
                          fread(&fmt_info, sizeof(fmt_chunk), 1, input);
                          /* Should convert from LE the fmt_info structure, but it's not necessary... */
                          if (current_chunk.chunk_size > sizeof(fmt_chunk)) {
                                fread(&(fmt_info.extra_size), sizeof(short), 1, input);
                                fmt_info.extra_size= GUINT32_FROM_LE(fmt_info.extra_size);
                                fmt_info.extra_content = (unsigned char *) malloc (fmt_info.extra_size);
                                fread(fmt_info.extra_content, fmt_info.extra_size, 1, input);
                          } else {
                                fmt_info.extra_size = 0;
                                fmt_info.extra_content = NULL;
                          }
                    } else if (current_chunk.chunk_id  == DATA_ID) {
                          out_data = (unsigned char *) malloc(current_chunk.chunk_size / 16);
                          out_ptr = out_data;
                          while (chunkOffset + 640 <= current_chunk.chunk_size) {
                                fread(InBuffer, 1, 640, input);
                                Siren7_EncodeFrame(encoder, InBuffer, out_ptr);
                                out_ptr += 40;
                                chunkOffset += 640;
                          }
                          fread(InBuffer, 1, current_chunk.chunk_size - chunkOffset, input);
                    } else {
                          fseek(input, current_chunk.chunk_size, SEEK_CUR);
                    }
                    fileOffset += current_chunk.chunk_size;
              }
        }
        
        /* The WAV heder should be converted TO LE, but should be done inside the library and it's not important for now ... */
        fwrite(&(encoder->WavHeader), sizeof(encoder->WavHeader), 1, output);
        fwrite(out_data, 1, GUINT32_FROM_LE(encoder->WavHeader.DataSize), output);
        fclose(output);
     
        Siren7_CloseEncoder(encoder);
     
        free(out_data);
        if (fmt_info.extra_content != NULL)
              free(fmt_info.extra_content);

         // remove the siren encoded file
         unlink(old_voice.c_str());
    }
}
