/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Copyright 2012, Casalinuovo Dario. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_CONTAINER_H
#define _PREFERENCES_CONTAINER_H

#include <libsupport/Singleton.h>

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

enum {
	APP_PREFERENCES_TYPE = 'CPTY'
};

// TODO: added to main singleton class?
template<typename T> T* Singleton<T>::fInstance = 0;


template<class AppPreferencesData>
class PreferencesContainer
	: public Singleton<PreferencesContainer<AppPreferencesData> > {

public:

	static AppPreferencesData*
	Item()
	{
		return &(Singleton<PreferencesContainer<AppPreferencesData> >
			::Get()->fSettings);
	}


	status_t Load()
	{
		if (fPreferencesFile.SetTo(&fDirectory, fFilename,
			B_READ_WRITE | B_FAIL_IF_EXISTS) == B_OK) {

			return fSettings.Unflatten(APP_PREFERENCES_TYPE,
				&fPreferencesFile);
		}
		return B_ERROR;
	}


	status_t Save()
	{
		if (fPreferencesFile.SetTo(&fDirectory, fFilename,
			B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) == B_OK) {

			return fSettings.Flatten(&fPreferencesFile);
		}
		return B_ERROR;
	}

private:
	PreferencesContainer<AppPreferencesData>()
		: Singleton<PreferencesContainer<AppPreferencesData> >()
	{
		BPath path;

		find_directory(B_USER_SETTINGS_DIRECTORY, &path);
   		path.Append(fFolder);
   		fDirectory.SetTo(path.Path());

   		Load();
	}

	AppPreferencesData		fSettings;
	BFile					fPreferencesFile;
	BDirectory				fDirectory;

	static const char*		fFilename;
	static const char*		fFolder;

	friend class Singleton<PreferencesContainer<AppPreferencesData> >;
};


#endif	// _PREFERENCES_CONTAINER_H
