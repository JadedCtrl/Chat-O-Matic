/*
 * Copyright 2010-2011, Pier Luigi Fiorini. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 *		Pier Luigi Fiorini, pierluigi.fiorini@gmail.com
 */

#include <Bitmap.h>
#include <Path.h>

#include <libinterface/BitmapUtils.h>

#include "CayaProtocol.h"
#include "CayaProtocolAddOn.h"


CayaProtocolAddOn::CayaProtocolAddOn(image_id image, const char* path, int32 subProto)
	:
	fImage(image),
	fPath(path),
	fIcon(NULL),
	fProtoIndex(subProto)
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
	return ProtocolAt(fProtoIndex);
}


CayaProtocol*
CayaProtocolAddOn::ProtocolAt(int32 i) const
{
	CayaProtocol* proto = fGetProtocol(i);
	proto->SetPath(BPath(fPath.String()));
	return proto;
}


int32
CayaProtocolAddOn::CountProtocols() const
{
	return fCountProtocols();
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


uint32
CayaProtocolAddOn::Version() const
{
	return fVersion;
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
	uint32 (*version)();

	fStatus = B_OK;

	if (get_image_symbol(fImage, "protocol_at", B_SYMBOL_TYPE_TEXT,
		(void**)&fGetProtocol) != B_OK) {
		unload_add_on(fImage);
		fStatus = B_ERROR;
		return;
	}

	if (get_image_symbol(fImage, "protocol_count", B_SYMBOL_TYPE_TEXT,
		(void**)&fCountProtocols) != B_OK) {
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

	if (get_image_symbol(fImage, "version", B_SYMBOL_TYPE_TEXT,
		(void**)&version) != B_OK) {
		unload_add_on(fImage);
		fStatus = B_ERROR;
		return;
	}

	fSignature = signature();
	fFriendlySignature = friendly_signature();
	fVersion = version();
}
