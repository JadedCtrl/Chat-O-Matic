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
 * Contributor(s): Rene Gollent
 *                 Todd Lair
 */
 
#include <ctype.h>

#include "URLCrunch.h"

URLCrunch::URLCrunch (const char *data, int32 len)
	: buffer (""),
	  current_pos (0)
{
	buffer.Append (data, len);
}

URLCrunch::~URLCrunch (void)
{
}

int32
URLCrunch::Crunch (BString *url)
{
	if (current_pos >= buffer.Length())
		return B_ERROR;

	const int32 tagNum = 7;
	const char *tags[tagNum] =
	{
		"http://",
		"https://",
		"www.",
		"ftp://",
		"ftp.",
		"file:/",
		"mailto:"
	};

	int32 marker (buffer.Length());
	int32 pos (current_pos);
	int32 url_length (0);
	int32 markers[tagNum];
	int32 i(0);

	for (i = 0; i < tagNum; ++i)
		markers[i] = buffer.IFindFirst (tags[i], pos);

	for (i = 0; i < tagNum; ++i)
	
		if (markers[i] != B_ERROR
		&&  markers[i] < marker)
		{
			url_length = markers[i] + strlen(tags[i]);
			
			url_length += strcspn (buffer.String() + url_length, " \t\n|\\<>\")(][}{;'*^");


			int len (strlen (tags[i]));

			if (url_length - markers[i] > len
			&& (isdigit (buffer[markers[i] + len])
			||  isalpha (buffer[markers[i] + len])))
			{
				marker = markers[i];
				pos = url_length + 1;
				url_length -= marker;
			}
			else
				pos = markers[i] + 1;
		}
		
		if (marker < buffer.Length())
		{
			*url = "";

			url->Append (buffer.String() + marker, url_length);
		}

		current_pos = pos;

	return marker < buffer.Length() ? marker : B_ERROR;
}
