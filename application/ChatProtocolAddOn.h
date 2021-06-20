/*
 * Copyright 2010-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _APP_PROTOCOL_ADDON_H
#define _APP_PROTOCOL_ADDON_H

#include <image.h>

#include <String.h>

class BBitmap;
class ChatProtocol;


class ChatProtocolAddOn {
public:
					ChatProtocolAddOn(image_id image, const char* path,
						int32 subProto=0);

	status_t		InitCheck() const;

	const char*		Path() const;

	ChatProtocol*	Protocol() const;
	ChatProtocol*	ProtocolAt(int32 i) const;

	int32			CountProtocols() const;

	const char*		Signature() const;
	const char*		FriendlySignature() const;
	BBitmap*		Icon() const;

	const char*		ProtoSignature() const;
	const char*		ProtoFriendlySignature() const;
	BBitmap*		ProtoIcon() const;

	uint32			Version() const;

private:
	image_id		fImage;
	BString			fPath;
	ChatProtocol*	(*fGetProtocol)(int32 i);
	int32			(*fCountProtocols)();
	int32			fProtoIndex;
	uint32			fVersion;
	BString			fSignature;
	BString			fFriendlySignature;
	BBitmap*		fIcon;
	status_t		fStatus;

	void			_Init();
};

#endif	// _APP_PROTOCOL_ADDON_H
