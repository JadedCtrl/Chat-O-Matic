/*
 * Copyright 2010, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Bitmap.h>

#include <libinterface/BitmapUtils.h>

#include "CayaProtocol.h"
#include "CayaProtocolAddOn.h"


CayaProtocolAddOn::CayaProtocolAddOn(image_id image, const char* path)
	:
	fImage(image),
	fPath(path),
	fIcon(NULL)
{
	_Init();
}


status_t
CayaProtocolAddOn::InitCheck() const
{
	return fStatus;
}


const char*
CayaProtocolAddOn::Path() const
{
	return fPath.String();
}


CayaProtocol*
CayaProtocolAddOn::Protocol() const
{
	return fGetProtocol();
}


const char*
CayaProtocolAddOn::Signature() const
{
	return fSignature.String();
}


const char*
CayaProtocolAddOn::FriendlySignature() const
{
	return fFriendlySignature.String();
}


BBitmap*
CayaProtocolAddOn::Icon() const
{
	return ReadNodeIcon(fPath, B_LARGE_ICON, true);
}


void
CayaProtocolAddOn::_Init()
{
	const char* (*signature)();
	const char* (*friendly_signature)();

	fStatus = B_OK;

	if (get_image_symbol(fImage, "protocol", B_SYMBOL_TYPE_TEXT,
		(void**)&fGetProtocol) != B_OK) {
		unload_add_on(fImage);
		fStatus = B_ERROR;
		return;
	}

	if (get_image_symbol(fImage, "signature", B_SYMBOL_TYPE_TEXT,
		(void**)&signature) != B_OK) {
		unload_add_on(fImage);
		fStatus = B_ERROR;
		return;
	}

	if (get_image_symbol(fImage, "friendly_signature", B_SYMBOL_TYPE_TEXT,
		(void**)&friendly_signature) != B_OK) {
		unload_add_on(fImage);
		fStatus = B_ERROR;
		return;
	}

	fSignature = signature();
	fFriendlySignature = friendly_signature();
}
