/*
 * Copyright 2010-2011, Pier Luigi Fiorini. All rights reserved.
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
					CayaProtocolAddOn(image_id image, const char* path,
						int32 subProto=0);

	status_t		InitCheck() const;

	const char*		Path() const;

	CayaProtocol*	Protocol() const;
	CayaProtocol*	ProtocolAt(int32 i) const;

	int32			CountProtocols() const;

	const char*		Signature() const;
	const char*		FriendlySignature() const;
	uint32			Version() const;

	BBitmap*		Icon() const;

private:
	image_id		fImage;
	BString			fPath;
	CayaProtocol*	(*fGetProtocol)(int32 i);
	int32			(*fCountProtocols)();
	int32			fProtoIndex;
	uint32			fVersion;
	BString			fSignature;
	BString			fFriendlySignature;
	BBitmap*		fIcon;
	status_t		fStatus;

	void			_Init();
};

#endif	// _CAYA_PROTOCOL_ADDON_H
