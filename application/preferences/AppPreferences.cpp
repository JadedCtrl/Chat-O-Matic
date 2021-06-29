/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#include "AppPreferences.h"
#include "Cardie.h"

#include <string.h>
#include <stdlib.h>

template<> const char* AppPreferences::fFolder = APP_NAME;
template<> const char* AppPreferences::fFilename = "preferences";

/* TODO update _Add* methods to 
 don't take the BPositionIO argument
 and to automatically increase 
 length counters. For example
 the _AddBool() method, should
 take the BPositionIO argument from
 a private class value, and increase
 the the size of another class value
 respectively. This way the api looks better
 and the possibility of bugs related to 
 size become very minimal : ).
*/

AppPreferencesData::AppPreferencesData()
	:
	MoveToCurrentWorkspace(true),
	RaiseOnMessageReceived(false),
	RaiseUserIsTyping(false),
	MarkUnreadWindow(true),
	HideDeskbar(false),
	DisableReplicant(true),
	IgnoreEmoticons(true),
	NotifyProtocolStatus(true),
	NotifyContactStatus(false),
	NotifyNewMessage(true),
	HideOffline(true),
	DisableQuitConfirm(true)
{
}


AppPreferencesData::~AppPreferencesData()
{
}


bool
AppPreferencesData::IsFixedSize() const
{
	return false;
}


type_code
AppPreferencesData::TypeCode() const
{
	return APP_PREFERENCES_TYPE;
}


bool
AppPreferencesData::AllowsTypeCode(type_code code) const
{
	if (code == APP_PREFERENCES_TYPE)
		return true;

	return false;
}


ssize_t
AppPreferencesData::FlattenedSize() const
{
	// NOTE add the size of every settings
	// you added.

	ssize_t size = sizeof(bool) * 11;

	return size;
}


status_t
AppPreferencesData::Flatten(BPositionIO* flatData) const
{
	if (flatData == NULL)
		return B_BAD_VALUE;

	// Write our type code
	type_code code = APP_PREFERENCES_TYPE;
	flatData->Write(&code, sizeof(type_code));

	// Behaviour
	_AddBool(flatData, MoveToCurrentWorkspace);
	_AddBool(flatData, RaiseOnMessageReceived);
	_AddBool(flatData, RaiseUserIsTyping);
	_AddBool(flatData, MarkUnreadWindow);

	_AddBool(flatData, NotifyProtocolStatus);
	_AddBool(flatData, NotifyContactStatus);
	_AddBool(flatData, NotifyNewMessage);
	_AddBool(flatData, DisableQuitConfirm);

	// Replicant
	_AddBool(flatData, HideDeskbar);
	_AddBool(flatData, DisableReplicant);

	// Chat window
	_AddBool(flatData, IgnoreEmoticons);
	
	// Contact list
	_AddBool(flatData, HideOffline);

	// Usage example for strings :
	// _AddString(flatData, yourBString.String());

	// NOTE : The order is very important, Unflatten and Flatten
	// classes should read/write the values in the same order.

	return B_OK;
}


status_t
AppPreferencesData::Flatten(void* buffer, ssize_t size) const
{
	if (buffer == NULL)
		return B_BAD_VALUE;

	BMemoryIO flatData(buffer, size);
	return Flatten(&flatData, size);
}



status_t
AppPreferencesData::Unflatten(type_code code, const void* buffer, ssize_t size)
{
	if (buffer == NULL)
		return B_BAD_VALUE;

	BMemoryIO flatData(buffer, size);
	return Unflatten(code, &flatData);
}


status_t
AppPreferencesData::Unflatten(type_code code, BPositionIO* flatData)
{
	if (code != APP_PREFERENCES_TYPE || flatData == NULL)
		return B_BAD_VALUE;

	// Reading our type code
	type_code typeCode;
	flatData->Read(&typeCode, sizeof(type_code));

	// checking if the typecode is correct
	if (code != typeCode)
		return B_BAD_VALUE;

	// Behaviour
	MoveToCurrentWorkspace = _ReadBool(flatData);
	RaiseOnMessageReceived = _ReadBool(flatData);
	RaiseUserIsTyping = _ReadBool(flatData);
	MarkUnreadWindow = _ReadBool(flatData);

	NotifyProtocolStatus = _ReadBool(flatData);
	NotifyContactStatus = _ReadBool(flatData);
	NotifyNewMessage = _ReadBool(flatData);
	DisableQuitConfirm = _ReadBool(flatData);

	// Replicant
	HideDeskbar = _ReadBool(flatData);
	DisableReplicant = _ReadBool(flatData);

	// Chat window
	IgnoreEmoticons = _ReadBool(flatData);
	
	// Contact list
	HideOffline = _ReadBool(flatData);

	// Usage example for strings :
	// const char* str = _ReadString(flatData);
	// yourBString.SetTo(str);

	// NOTE : The order is very important, Unflatten and Flatten
	// classes should read/write the values in the same order.

	return B_OK;
}


void
AppPreferencesData::_AddBool(BPositionIO* data, bool value) const
{
	data->Write(&value, sizeof(value));
}


void
AppPreferencesData::_AddString(BPositionIO* data, const char* value) const
{
	size_t len = strlen(value);
	data->Write(&len, sizeof(size_t));
	data->Write(value, len);
}


bool
AppPreferencesData::_ReadBool(BPositionIO* data)
{
	bool ret;
	data->Read(&ret, sizeof(bool));
	return ret;
}


const char*
AppPreferencesData::_ReadString(BPositionIO* data)
{
	size_t len;
	data->Read(&len, sizeof(size_t));
	char* ret = new char[len];
	data->Read(ret, len);

	return ret;
}
