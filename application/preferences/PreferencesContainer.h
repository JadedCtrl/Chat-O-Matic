/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_CONTAINER_H
#define _PREFERENCES_CONTAINER_H

#include <libsupport/Singleton.h>

// TODO: added to main singleton class?
template<typename T> T* Singleton<T>::fInstance = 0;


template<class SettingsType>
class PreferencesContainer : public Singleton<PreferencesContainer<SettingsType> > {

public:
	static const char* fFilename;

	static SettingsType*
	Item()
	{
		return &Get()->fSettings;
	}

	// TODO:
	// status_t Save();
	// status_t Load();

private:
	SettingsType		fSettings;

};


#endif	// _PREFERENCES_CONTAINER_H
