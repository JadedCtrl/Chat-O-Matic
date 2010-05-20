/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
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
	/* Returns the image corresponding to the which constant */
	static	BBitmap*			GetImage(BString fullPath,
									BString symbolicName);

	static	void				AddImage(BString name, BBitmap* which);
	static	void				DeleteImage(BString name);

	/* Frees the singleton instance of the cache, must be
	 * called when the application quits.
	 */
	static	void				Release();

protected:
								ImageCache();
								~ImageCache();

private:
	static	BBitmap*			LoadImage(const char* resourceName,
									const char*);

	static	ImageCache*			fInstance;
	KeyMap<BString, BBitmap*>	fBitmaps;
};

#endif	// _IMAGE_CACHE_H
