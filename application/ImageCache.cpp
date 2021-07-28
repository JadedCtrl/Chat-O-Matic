/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Copyright 2021, Jaidyn Levesque. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "ImageCache.h"

#include <AppDefs.h>
#include <Bitmap.h>
#include <Debug.h>
#include <Resources.h>
#include <TranslationUtils.h>

#include <libinterface/BitmapUtils.h>

#include "AppResources.h"
#include "Utils.h"


ImageCache* ImageCache::fInstance = NULL;


ImageCache::ImageCache()
{
	_LoadResource(kPersonIcon, "kPersonIcon");
	_LoadResource(kChatIcon, "kChatIcon");

	_LoadResource(kAwayReplicant, "kAwayReplicant");
	_LoadResource(kBusyReplicant, "kBusyReplicant");
	_LoadResource(kOfflineReplicant, "kOfflineReplicant");
	_LoadResource(kOnlineReplicant, "kOnlineReplicant");
}


ImageCache::~ImageCache()
{
	while (fBitmaps.CountItems()) {
		BBitmap* bit = fBitmaps.ValueFor(0);
		delete bit;
	}
}


ImageCache*
ImageCache::Get()
{
	if (fInstance == NULL) {
		fInstance = new ImageCache();
	}
	return fInstance;
}


BBitmap*
ImageCache::GetImage(const char* keyName)
{
	// Loads the bitmap if found
	bool found;
	BBitmap* bitmap = fBitmaps.ValueFor(BString(keyName), &found);

	if (found == true)
		return bitmap;
	return NULL;
}


void
ImageCache::AddImage(BString name, BBitmap* which)
{
	fBitmaps.AddItem(name, which);
}


void
ImageCache::DeleteImage(BString name)
{
	BBitmap* bitmap = fBitmaps.ValueFor(name);
	if (bitmap) {
		fBitmaps.RemoveItemFor(name);
		delete bitmap;	
	}
}


void
ImageCache::Release()
{
	if (fInstance != NULL) {
		delete fInstance;
		fInstance = NULL;
	}
}


void
ImageCache::_LoadResource(int identifier, const char* key)
{
	BResources* res = ChatResources();
	BBitmap* bitmap = IconFromResources(res, identifier, B_LARGE_ICON);
	if (bitmap != NULL && bitmap->IsValid() == true)
		fBitmaps.AddItem(BString(key), bitmap);
}
