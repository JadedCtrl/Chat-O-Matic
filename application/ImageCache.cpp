/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include "ImageCache.h"

#include <AppDefs.h>
#include <Bitmap.h>
#include <Debug.h>
#include <TranslationUtils.h>

ImageCache* ImageCache::fInstance = NULL;


ImageCache::ImageCache()
{
}


ImageCache::~ImageCache()
{
	while (fBitmaps.CountItems()) {
		BBitmap* bit = fBitmaps.ValueFor(0);
		delete bit;
	}
}


BBitmap*
ImageCache::GetImage(BString which, BString name)
{
	if (fInstance == NULL)
		fInstance = new ImageCache();

	// Loads the bitmap if found
	bool found;
	BBitmap* bitmap = fInstance->fBitmaps.ValueFor(name, &found);

	if (!found) {
		bitmap = LoadImage(which.String(), name.String());
		if (bitmap)
			fInstance->fBitmaps.AddItem(name, bitmap);
		return bitmap;
	} else
		return bitmap;

	return NULL;
}


void
ImageCache::AddImage(BString name, BBitmap* which)
{
	if (fInstance == NULL)
		fInstance = new ImageCache();

	fInstance->fBitmaps.AddItem(name, which);
}


void
ImageCache::DeleteImage(BString name)
{
	if (fInstance == NULL)
		fInstance = new ImageCache();

	BBitmap* bitmap = fInstance->fBitmaps.ValueFor(name);	
	if (bitmap) {
		fInstance->fBitmaps.RemoveItemFor(name);
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


BBitmap*
ImageCache::LoadImage(const char* fullName, const char* shortName)
{
	BBitmap* bitmap = BTranslationUtils::GetBitmap(fullName);
	if (!bitmap)
		bitmap = BTranslationUtils::GetBitmap('PNG ', shortName);

	if (!bitmap)
		printf("ImageCache: Can't load bitmap! %s\n", fullName);
	return bitmap;
}
