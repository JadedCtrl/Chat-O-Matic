/* 
 * The contents of this file are subject to the Mozilla Public 
 * License Version 1.1 (the "License"); you may not use this file 
 * except in compliance with the License. You may obtain a copy of 
 * the License at http://www.mozilla.org/MPL/ 
 * 
 * Software distributed under the License is distributed on an "AS 
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or 
 * implied. See the License for the specific language governing 
 * rights and limitations under the License. 
 * 
 * The Original Code is Vision. 
 * 
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999, 2000, 2001 The Vision Team.  All Rights
 * Reserved.
 * 
 * Contributor(s): Wade Majors <wade@ezri.org>
 *                 Todd Lair
 *                 Andrew Bazan
 */
 
#ifndef _UTILITIES_H
#define _UTILITIES_H_

#include <String.h>

template<class T> class AutoDestructor {
  public:
    AutoDestructor(T *t)
    {
      fObject = t;
    }
    
    virtual ~AutoDestructor(void)
    {
      delete fObject;
    }
    
    void SetTo(T *t)
    {
      delete fObject;
      fObject = t;
    }
    
  private:
    T *fObject;  
};


class BMessage;
class BPoint;

BString      GetWord (const char *, int32);
BString      RestOfString (const char *, int32);
BString      GetNick (const char *);
BString      GetIdent (const char *);
BString      GetAddress (const char *);
BString      TimeStamp (void);
BString      ExpandKeyed (const char *, const char *, const char **);
BString      DurationString (int64);
BString      StringToURI (const char *);
const char   *RelToAbsPath (const char *);
BString      GetWordColon (const char *, int32);
int32        Get440Len (const char *);
uint16       CheckClickCount(BPoint, BPoint &, bigtime_t, bigtime_t &, int16 &);


#endif
