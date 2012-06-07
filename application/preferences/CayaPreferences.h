/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PREFERENCES_H
#define _CAYA_PREFERENCES_H

#include "PreferencesContainer.h"


class CayaPreferencesData : public BFlattenable {
public:
							CayaPreferencesData();
	virtual					~CayaPreferencesData();

	virtual	bool			IsFixedSize() const;
	virtual	type_code		TypeCode() const;
	virtual	bool			AllowsTypeCode(type_code code) const;
	virtual	ssize_t			FlattenedSize() const;

			status_t 		Flatten(BPositionIO* flatData) const;
	virtual	status_t		Flatten(void* buffer, ssize_t size) const;
	virtual	status_t		Unflatten(type_code code, const void* buffer,
								ssize_t size);
			status_t		Unflatten(type_code code, BPositionIO* flatData);

			bool 			MoveToCurrentWorkspace;
			bool 			RaiseOnMessageReceived;
			bool 			RaiseUserIsTyping;
			bool 			NotifyProtocolStatus;
			bool 			NotifyContactStatus;

			bool 			HideCayaDeskbar;
			bool 			DisableReplicant;

			bool 			IgnoreEmoticons;
private:
			void 			_AddBool(BPositionIO* data, bool value) const;
			void 			_AddString(BPositionIO* data,
								const char* value) const;

			bool			_ReadBool(BPositionIO* data);
			const char* 	_ReadString(BPositionIO* data);
};

typedef PreferencesContainer<CayaPreferencesData> CayaPreferences;

#endif	// _CAYA_PREFERENCES_H
