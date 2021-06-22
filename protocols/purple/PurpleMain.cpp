/*
 Copyright 2021, Jaidyn Levesque <jadedctrl@teknik.io>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "PurpleProtocol.h"


extern "C" _EXPORT ChatProtocol* protocol_at(int32 i);
extern "C" _EXPORT int32 protocol_count();
extern "C" _EXPORT const char* signature();
extern "C" _EXPORT const char* friendly_signature();
extern "C" _EXPORT uint32 version();


ChatProtocol*
protocol_at(int32 i)
{
	if (i == 0)
			return (ChatProtocol*)new PurpleProtocol();
	return NULL;
}


int32
protocol_count()
{
	return 1;
}


const char*
signature()
{
	return "purple";
}


const char*
friendly_signature()
{
	return "Purple";
}


uint32
version()
{
	return APP_VERSION_1_PRE_ALPHA_1;
}


