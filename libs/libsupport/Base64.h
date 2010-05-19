/*
 * Copyright 2009, Pier Luigi Fiorini.
 * Copyright 2004-2009, René Nyffenegge
 * Distributed under the terms of the MIT License.
 */
#ifndef _BASE_64_H
#define _BASE_64_H

#include <String.h>

class Base64 {
public:
	static	bool	IsBase64(uchar c);
	static	BString	Encode(const uchar* data, size_t length);
	static	BString	Decode(const BString& data);

private:
	Base64();
};

#endif	// _BASE_64_H
