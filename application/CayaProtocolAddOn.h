/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _CAYA_PROTOCOL_ADDON_H
#define _CAYA_PROTOCOL_ADDON_H

#include <image.h>

#include <String.h>

class BBitmap;

class CayaProtocol;

class CayaProtocolAddOn {
public:
					CayaProtocolAddOn(image_id image, const char* path);

	status_t		InitCheck() const;

	const char*		Path() const;

	CayaProtocol*	Protocol() const;
	const char*		Signature() const;
	const char*		FriendlySignature() const;
	BBitmap*		Icon() const;

private:
	image_id		fImage;
	BString			fPath;
	CayaProtocol*	(*fGetProtocol)();
	BString			fSignature;
	BString			fFriendlySignature;
	BBitmap*		fIcon;
	status_t		fStatus;

	void			_Init();
};

#endif	// _CAYA_PROTOCOL_ADDON_H
