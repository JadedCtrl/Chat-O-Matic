/*
 * Copyright 2009-2011, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _IMAGE_CACHE_H
#define _IMAGE_CACHE_H

#include <SupportDefs.h>
#include <String.h>

#include <libsupport/KeyMap.h>

class BBitmap;

class ImageCache {
public:
	static	ImageCache*			Get();

	/* Returns the image corresponding to the which constant */
			BBitmap*			GetImage(const char* keyName);

			void				AddImage(BString name, BBitmap* which);
			void				DeleteImage(BString name);

	/* Frees the singleton instance of the cache, must be
	 * called when the application quits.
	 */
	static	void				Release();

protected:
								ImageCache();
								~ImageCache();

private:
			void				_LoadResource(int identifier, const char* key);

	static	ImageCache*			fInstance;
	KeyMap<BString, BBitmap*>	fBitmaps;
};


#endif	// _IMAGE_CACHE_H

