/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _APP_PREFERENCES_H
#define _APP_PREFERENCES_H

#include "PreferencesContainer.h"


class AppPreferencesData : public BFlattenable {
public:
							AppPreferencesData();
	virtual					~AppPreferencesData();

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
			bool			MarkUnreadWindow;
			bool 			NotifyProtocolStatus;
			bool 			NotifyContactStatus;
			bool			NotifyNewMessage;

			bool 			HideDeskbar;
			bool 			DisableReplicant;
			bool			DisableQuitConfirm;

			bool 			IgnoreEmoticons;
			
			bool			HideOffline;
private:
			void 			_AddBool(BPositionIO* data, bool value) const;
			void 			_AddString(BPositionIO* data,
								const char* value) const;

			bool			_ReadBool(BPositionIO* data);
			const char* 	_ReadString(BPositionIO* data);
};

typedef PreferencesContainer<AppPreferencesData> AppPreferences;

#endif	// _APP_PREFERENCES_H
