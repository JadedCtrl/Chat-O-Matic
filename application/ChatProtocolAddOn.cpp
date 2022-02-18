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

#include "ChatProtocol.h"
#include "ChatProtocolAddOn.h"


ChatProtocolAddOn::ChatProtocolAddOn(image_id image, const char* path, int32 subProto)
	:
	fImage(image),
	fPath(path),
	fIcon(NULL),
	fProtoIndex(subProto)
{
	_Init();
}


status_t
ChatProtocolAddOn::InitCheck() const
{
	return fStatus;
}


const char*
ChatProtocolAddOn::Path() const
{
	return fPath.String();
}


ChatProtocol*
ChatProtocolAddOn::Protocol() const
{
	return ProtocolAt(fProtoIndex);
}


ChatProtocol*
ChatProtocolAddOn::ProtocolAt(int32 i) const
{
	ChatProtocol* proto = fGetProtocol(i);
	proto->SetAddOnPath(fPath.String());
	return proto;
}


int32
ChatProtocolAddOn::CountProtocols() const
{
	return fCountProtocols();
}


const char*
ChatProtocolAddOn::Signature() const
{
	return fSignature.String();
}


const char*
ChatProtocolAddOn::FriendlySignature() const
{
	return fFriendlySignature.String();
}


BBitmap*
ChatProtocolAddOn::Icon() const
{
	return ReadNodeIcon(fPath, B_LARGE_ICON, true);
}


const char*
ChatProtocolAddOn::ProtoSignature() const
{
	ChatProtocol* proto = Protocol();
	const char* signature = proto->Signature();
	delete proto;
	return signature;
}


const char*
ChatProtocolAddOn::ProtoFriendlySignature() const
{
	ChatProtocol* proto = Protocol();
	const char* signature = proto->FriendlySignature();
	delete proto;
	return signature;
}


BBitmap*
ChatProtocolAddOn::ProtoIcon() const
{
	ChatProtocol* proto = Protocol();
	BBitmap* icon = proto->Icon();
	delete proto;
	return icon;
}


uint32
ChatProtocolAddOn::Version() const
{
	return fVersion;
}


void
ChatProtocolAddOn::_Init()
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
