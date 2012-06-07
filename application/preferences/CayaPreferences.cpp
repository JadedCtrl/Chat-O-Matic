/*
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#include "CayaPreferences.h"

#include <string.h>
#include <stdlib.h>

template<> const char* CayaPreferences::fFolder = "Caya";
template<> const char* CayaPreferences::fFilename = "preferences";


CayaPreferencesData::CayaPreferencesData()
	:
	MoveToCurrentWorkspace(true),
	RaiseOnMessageReceived(false),
	RaiseUserIsTyping(false),
	HideCayaDeskbar(false),
	DisableReplicant(true),
	IgnoreEmoticons(false),
	NotifyProtocolStatus(true),
	NotifyContactStatus(false)
{
}


CayaPreferencesData::~CayaPreferencesData()
{
}


bool
CayaPreferencesData::IsFixedSize() const
{
	return false;
}


type_code
CayaPreferencesData::TypeCode() const
{
	return CAYA_PREFERENCES_TYPE;
}


bool
CayaPreferencesData::AllowsTypeCode(type_code code) const
{
	if (code == CAYA_PREFERENCES_TYPE)
		return true;

	return false;
}


ssize_t
CayaPreferencesData::FlattenedSize() const
{
	// NOTE add the size of every settings
	// you added.

	ssize_t size = sizeof(bool) * 8;

	return size;
}


status_t
CayaPreferencesData::Flatten(BPositionIO* flatData) const
{
	if (flatData == NULL)
		return B_BAD_VALUE;

	// Write our type code
	type_code code = CAYA_PREFERENCES_TYPE;
	flatData->Write(&code, sizeof(type_code));

	// Behaviour
	_AddBool(flatData, MoveToCurrentWorkspace);
	_AddBool(flatData, RaiseOnMessageReceived);
	_AddBool(flatData, RaiseUserIsTyping);

	_AddBool(flatData, NotifyProtocolStatus);
	_AddBool(flatData, NotifyContactStatus);

	// Replicant
	_AddBool(flatData, HideCayaDeskbar);
	_AddBool(flatData, DisableReplicant);

	// Chat window
	_AddBool(flatData, IgnoreEmoticons);

	// Usage example for strings :
	// _AddString(flatData, yourBString.String());

	// NOTE : The order is very important, Unflatten and Flatten
	// classes should read/write the values in the same order.

	_AddString(flatData, "str");

	return B_OK;
}


status_t
CayaPreferencesData::Flatten(void* buffer, ssize_t size) const
{
	if (buffer == NULL)
		return B_BAD_VALUE;

	BMemoryIO flatData(buffer, size);
	return Flatten(&flatData, size);
}



status_t
CayaPreferencesData::Unflatten(type_code code, const void* buffer, ssize_t size)
{
	if (buffer == NULL)
		return B_BAD_VALUE;

	BMemoryIO flatData(buffer, size);
	return Unflatten(code, &flatData);
}


status_t
CayaPreferencesData::Unflatten(type_code code, BPositionIO* flatData)
{
	if (code != CAYA_PREFERENCES_TYPE || flatData == NULL)
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

	NotifyProtocolStatus = _ReadBool(flatData);
	NotifyContactStatus = _ReadBool(flatData);

	// Replicant
	HideCayaDeskbar = _ReadBool(flatData);
	DisableReplicant = _ReadBool(flatData);

	// Chat window
	IgnoreEmoticons = _ReadBool(flatData);

	// Usage example for strings :
	// const char* str = _ReadString(flatData);
	// yourBString.SetTo(str);

	// NOTE : The order is very important, Unflatten and Flatten
	// classes should read/write the values in the same order.

	return B_OK;
}


void
CayaPreferencesData::_AddBool(BPositionIO* data, bool value) const
{
	data->Write(&value, sizeof(value));
}


void
CayaPreferencesData::_AddString(BPositionIO* data, const char* value) const
{
	size_t len = strlen(value);
	data->Write(&len, sizeof(size_t));
	data->Write(value, len);
}


bool
CayaPreferencesData::_ReadBool(BPositionIO* data)
{
	bool ret;
	data->Read(&ret, sizeof(bool));
	return ret;
}


const char*
CayaPreferencesData::_ReadString(BPositionIO* data)
{
	size_t len;
	char* ret;
	data->Read(&len, sizeof(size_t));
	ret = new char[len];
	data->Read(ret, len);

	return ret;
}
