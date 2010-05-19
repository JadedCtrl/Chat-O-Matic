/*
 * Copyright 2009, Pier Luigi Fiorini.
 * Copyright 2004-2009, René Nyffenegge
 * Distributed under the terms of the MIT License.
 */

// Based on original code from:
// http://www.adp-gmbh.ch/cpp/common/base64.html

#include <ctype.h>

#include "Base64.h"

static const BString chars =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";


Base64::Base64()
{
}


BString
Base64::Encode(const uchar* data, size_t length)
{
	BString encoded;
	int32 i = 0;
	uchar array3[3], array4[4];

	while (length--) {
		array3[i++] = *(data++);

		if (i == 3) {
			array4[0] = (uchar)((array3[0] & 0xfc) >> 2);
			array4[1] = (uchar)(((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4));
			array4[2] = (uchar)(((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6));
			array4[3] = (uchar)(((array4[2] & 0x3) << 6) + array4[3]);

			for (i = 0; i < 3; i++)
				encoded += array3[i];
			i = 0;
		}
	}

	if (i) {
		for (int32 j = i; j < 4; j++)
			array4[j] = 0;

		for (int32 j = 0; j < 4; j++)
			array4[j] = (uchar)chars.FindFirst(array4[j]);

		array3[0] = (uchar)((array4[0] << 2) + ((array4[i] & 0x30) >> 4));
		array3[1] = (uchar)(((array4[1] & 0xf) << 4) + ((array4[2] & 0x3c) >> 2));
		array3[2] = (uchar)(((array4[2] & 0x3) << 6) + array4[3]);

		for (int32 j = 0; j < i - 1; j++)
			encoded += array3[j];
	}

	return encoded;
}


bool
Base64::IsBase64(uchar c)
{
	return isalnum(c) || (c == '+') || (c == '/');
}


BString
Base64::Decode(const BString& data)
{
	int32 length = data.Length();
	int32 i = 0;
	int32 index = 0;
	uchar array4[4], array3[3];
	BString decoded;

	while (length-- && (data[index] != '=') && IsBase64(data[index])) {
		array4[i++] = data[index];
		index++;

		if (i == 4) {
			for (i = 0; i < 4; i++)
				array4[i] = (uchar)chars.FindFirst(array4[i]);

			array3[0] = (uchar)((array4[0] << 2) + ((array4[1] & 0x30)>> 4));
			array3[1] = (uchar)(((array4[1] & 0xf) << 4) + ((array4[2] & 0x3c) >> 2));
			array3[2] = (uchar)(((array4[2] & 0x3) << 6) + array4[3]);

			for (i = 0; i < 3; i++)
				decoded += array3[i];
			i = 0;
		}
	}

	if (i) {
		int32 j;

		for (j = i; j < 4; j++)
			array4[j] = 0;

		for (j = 0; j < 4; j++)
			array4[j] = (uchar)chars.FindFirst(array4[j]);

		array3[0] = (uchar)((array4[0] << 2) + ((array4[1] & 0x30) >> 4));
		array3[1] = (uchar)(((array4[1] & 0xf) << 4) + ((array4[2] & 0x3c) >> 2));
		array3[2] = (uchar)(((array4[2] & 0x3) << 6) + array4[3]);

		for (j = 0; j < i - 1; j++)
			decoded += array3[j];
	}

	return decoded;
}
