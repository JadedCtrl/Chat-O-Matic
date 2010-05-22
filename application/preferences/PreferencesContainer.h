/*
 * Copyright 2010, Oliver Ruiz Dorantes. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _PREFERENCES_CONTAINER_H
#define _PREFERENCES_CONTAINER_H

#include <libsupport/Singleton.h>

#include <Directory.h>
#include <File.h>
#include <FindDirectory.h>
#include <Path.h>

// TODO: added to main singleton class?
template<typename T> T* Singleton<T>::fInstance = 0;


template<class SettingsType>
class PreferencesContainer : public Singleton<PreferencesContainer<SettingsType> > {

public:

	static SettingsType*
	Item()
	{
		return &(Singleton<PreferencesContainer<SettingsType> >::Get()->fSettings);
	}


	status_t Load()
	{
		if (fPreferencesFile.SetTo(&fDirectory, fFilename,
			B_READ_WRITE | B_FAIL_IF_EXISTS) == B_OK) {

			// reset the file pointer
			fPreferencesFile.Seek(0, SEEK_SET);

			if (fPreferencesFile.Read(&fSettings, sizeof(SettingsType)) > 0)
				return B_OK;
		}

		return B_ERROR;
	}


	status_t Save()
	{
		if (fPreferencesFile.SetTo(&fDirectory, fFilename,
			B_READ_WRITE | B_CREATE_FILE | B_ERASE_FILE) == B_OK) {

			if (fPreferencesFile.Write(&fSettings, sizeof(SettingsType)) > 0)
				return B_OK;
		}

		return B_ERROR;
	}

private:
	PreferencesContainer<SettingsType>()
		: Singleton<PreferencesContainer<SettingsType> >()
	{
		BPath path;

		find_directory(B_USER_SETTINGS_DIRECTORY, &path);
   		path.Append(fFolder);
   		fDirectory.SetTo(path.Path());

   		Load();
	}

	friend class Singleton<PreferencesContainer<SettingsType> >;

	SettingsType		fSettings;
	BFile				fPreferencesFile;
	BDirectory			fDirectory;

	static const char*	fFilename;
	static const char*	fFolder;
};


#endif	// _PREFERENCES_CONTAINER_H
