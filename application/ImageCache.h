/*
 * Copyright 2009, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef ImageCache_H
#define ImageCache_H

 /**	
 *		ImageCache.	
 *		@author	Andrea Anzani.   
 */
 
class BBitmap;

#include <SupportDefs.h>
#include <String.h>

#include <libsupport/KeyMap.h>

class ImageCache
{

protected:						// Constructor/Destructor

								ImageCache();

								~ImageCache();

public:							// Operations

	/** Returns the image corresponding to the which constant */
	static  BBitmap *		GetImage( BString fullPath , BString symbolicName);


	static	 void AddImage(BString name,BBitmap* which);
	static	 void DeleteImage(BString name);

	/** Frees the singleton instance of the cache;
		Call this when app quits
	 */
	static void					Release();

private:					

	static BBitmap *	LoadImage(	const char *resourceName,const char *);
								// Class Data

	static ImageCache *		m_instance;

private:						// Instance Data


	KeyMap<BString,BBitmap*>	m_bitmaps;

};

#endif /* __C_ImageCache_H__ */
