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
 
#include <stdio.h>
#include <time.h>

#include <Application.h>
#include <Roster.h>
#include <Entry.h>
#include <Path.h>
#include <String.h>

const float doubleClickThresh = 6;

BString
GetWord (const char *cData, int32 wordNeeded)
{
  /*
   * Function purpose: Get word number {wordNeeded} from {cData}
   *                   (space delimited)
   */
   
  BString data (cData);
  BString buffer ("-9z99");
  int32 wordAt (1), place (0);

  while (wordAt != wordNeeded && place != B_ERROR)
  {
    if ((place = data.FindFirst ('\x20', place)) != B_ERROR)
      if (++place < data.Length()
      &&  data[place] != '\x20')
        ++wordAt;
  }

  if (wordAt == wordNeeded
  &&  place != B_ERROR
  &&  place < data.Length())
  {
    int32 end (data.FindFirst ('\x20', place));

    if (end == B_ERROR)
      end = data.Length();

    data.CopyInto (buffer, place, end - place);
  }

  return buffer;
}

BString
GetWordColon (const char *cData, int32 wordNeeded)
{
  /*
   * Function purpose: Get word number {wordNeeded} from {cData}
   *                   (colon delimited)
   */
   
  BString data (cData);
  BString buffer ("-9z99");
  int32 wordAt (1), place (0);

  while (wordAt != wordNeeded && place != B_ERROR)
  {
    if ((place = data.FindFirst (':', place)) != B_ERROR)
      if (++place < data.Length()
      &&  data[place] != ':')
        ++wordAt;
  }

  if (wordAt == wordNeeded
  &&  place != B_ERROR
  &&  place < data.Length())
  {
    int32 end (data.FindFirst (':', place));

    if (end == B_ERROR)
      end = data.Length();

    data.CopyInto (buffer, place, end - place);
  }

  return buffer;
}

BString
RestOfString (const char *cData, int32 wordStart)
{
  /*
   * Function purpose: Get word number {wordStart} from {cData}
   *                   append the rest of the string after {wordStart}
   *                   (space delimited)
   */

  BString data (cData);
  int32 wordAt (1), place (0);
  BString buffer ("-9z99");
	
  while (wordAt != wordStart && place != B_ERROR)
  {
    if ((place = data.FindFirst ('\x20', place)) != B_ERROR)
      if (++place < data.Length()
      &&  data[place] != '\x20')
      ++wordAt;
  }

  if (wordAt == wordStart
  &&  place != B_ERROR
  &&  place < data.Length())
    data.CopyInto (buffer, place, data.Length() - place);

  return buffer;
}

BString
GetNick (const char *cData)
{
  /*
   * Function purpose: Get nickname from {cData}
   *
   *  Expected format: nickname!user@host.name
   */
   
  BString data (cData);
  BString theNick;

  for (int32 i = 1; i < data.Length() && data[i] != '!' && data[i] != '\x20'; ++i)
    theNick += data[i];

  return theNick;
}

BString
GetIdent (const char *cData)
{
  /*
   * Function purpose: Get identname/username from {cData}
   *
   *  Expected format: nickname!user@host.name
   */
   
  BString data (GetWord(cData, 1));
  BString theIdent;
  int32 place[2];

  if ((place[0] = data.FindFirst ('!')) != B_ERROR
  &&  (place[1] = data.FindFirst ('@')) != B_ERROR)
  {
    ++(place[0]);
    data.CopyInto (theIdent, place[0], place[1] - place[0]);
  }
		
  return theIdent;
}

BString
GetAddress (const char *cData)
{
  /*
   * Function purpose: Get address/hostname from {cData}
   *
   *  Expected format: nickname!user@host.name
   */

  BString data (GetWord(cData, 1));
  BString address;
  int32 place;

  if ((place = data.FindFirst ('@')) != B_ERROR)
  {
    int32 length (data.FindFirst ('\x20', place));

    if (length == B_ERROR)
      length = data.Length();

    ++place;
    data.CopyInto (address, place, length - place);
  }

  return address;
}

BString
TimeStamp()
{
  /*
   * Function purpose: Return the timestamp string
   *
   */
   
//  if(!vision_app->GetBool ("timestamp"))
//    return "";
    
//  const char *ts_format (vision_app->GetString ("timestamp_format"));
	const char *ts_format = "[%H:%M]";

  time_t myTime (time (0));
  tm curTime;
  localtime_r (&myTime, &curTime);
	
  char tempTime[32];
  tempTime[strftime (tempTime, 31, ts_format, &curTime)] = '\0'; 
	
  return BString (tempTime).Append('\x20', 1);
}


BString
ExpandKeyed (
  const char *incoming,
  const char *keys,
  const char **expansions)
{
  BString buffer;

  while (incoming && *incoming)
  {
    if (*incoming == '$')
    {
      const char *place;

      ++incoming;

      if ((place = strchr (keys, *incoming)) != 0)
        buffer += expansions[place - keys];
      else
        buffer += *incoming;
    }
    else
      buffer += *incoming;

    ++incoming;
  }

  buffer += "\n";

  return buffer;
}

BString
StringToURI (const char *string)
{
  /*
   * Function purpose: Converts {string} to a URI safe format
   *
   */

  BString buffer (string);
  buffer.ToLower();
  buffer.ReplaceAll ("%",  "%25"); // do this first!
  buffer.ReplaceAll ("\n", "%20");
  buffer.ReplaceAll (" ",  "%20");
  buffer.ReplaceAll ("\"", "%22");
  buffer.ReplaceAll ("#",  "%23");  
  buffer.ReplaceAll ("@",  "%40");
  buffer.ReplaceAll ("`",  "%60");
  buffer.ReplaceAll (":",  "%3A");
  buffer.ReplaceAll ("<",  "%3C");
  buffer.ReplaceAll (">",  "%3E");
  buffer.ReplaceAll ("[",  "%5B");
  buffer.ReplaceAll ("\\", "%5C");
  buffer.ReplaceAll ("]",  "%5D");
  buffer.ReplaceAll ("^",  "%5E");
  buffer.ReplaceAll ("{",  "%7B");
  buffer.ReplaceAll ("|",  "%7C");
  buffer.ReplaceAll ("}",  "%7D");
  buffer.ReplaceAll ("~",  "%7E");    
  return buffer;
}

BString
DurationString (int64 value)
{
  /*
   * Function purpose: Return a duration string based on {value}
   *
   */
   
	BString duration;
	bigtime_t micro = value;
	bigtime_t milli = micro/1000;
	bigtime_t sec = milli/1000;
	bigtime_t min = sec/60;
	bigtime_t hours = min/60;
	bigtime_t days = hours/24;

	char message[512] = "";
	if (days)
		sprintf(message, "%Ld day%s ",days,days!=1?"s":"");
	
	if (hours%24)
		sprintf(message, "%s%Ld hr%s ",message, hours%24,(hours%24)!=1?"s":"");
	
	if (min%60)
		sprintf(message, "%s%Ld min%s ",message, min%60, (min%60)!=1?"s":"");

	sprintf(message, "%s%Ld.%Ld sec%s",message, sec%60, (milli%1000), (sec%60)!=1?"s":"");

	duration += message;

	return duration;
}


const char *
RelToAbsPath (const char *append_)
{
  app_info ai;
  be_app->GetAppInfo (&ai);
		
  BEntry entry (&ai.ref);
  BPath path;
  entry.GetPath (&path);
  path.GetParent (&path);
  path.Append (append_);
  
  return path.Path();  
}


int32
Get440Len (const char *cData)
{
  BString data (cData);

  if (data.Length() < 440)
    return data.Length();
  else
  {
    int32 place (data.FindLast ('\x20', 440));
    if (place == B_ERROR)
      return 440;
    return place;
  }
}

uint16
CheckClickCount(BPoint point, BPoint &lastClick, bigtime_t sysTime, bigtime_t &lastClickTime, int16 &clickCount)
{
  // check time and proximity
  BPoint delta = point - lastClick;

  bigtime_t timeDelta = sysTime - lastClickTime;

  bigtime_t doubleClickSpeed;
  get_click_speed(&doubleClickSpeed);

  lastClickTime = sysTime;

  if (timeDelta < doubleClickSpeed
    && fabs(delta.x) < doubleClickThresh
    && fabs(delta.y) < doubleClickThresh)
      return (++clickCount);

  lastClick = point;
  clickCount = 1;
  return clickCount;
}

