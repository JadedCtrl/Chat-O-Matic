/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */

#include <string.h>

#include "JabberElement.h"

JabberElement::JabberElement()
	: fName(""),
	fData(""),
	fAttr(NULL),
	fAttrCount(-1)
{
}


JabberElement::~JabberElement()
{
	Free();
}


void
JabberElement::SetName(const BString& name)
{
	fName = name;
}


BString
JabberElement::GetName() const
{
	return fName;
}


void
JabberElement::SetData(const BString& data)
{
	fData = data;
}


BString
JabberElement::GetData() const
{
	return fData;
}


void 
JabberElement::SetAttr(const char** attr)
{
	Free();
	if (attr) {
		const char** a = attr;

		fAttrCount = 0;

		while (*a) {
			fAttrCount++;
			a++;
		}

		fAttr = new char *[fAttrCount + 1];
		for (int32 i = 0; i < fAttrCount; i++) {
			fAttr[i] = new char[strlen(attr[i]) + 1];
			strcpy(fAttr[i], attr[i]);
		}
	}
}


const char**
JabberElement::GetAttr() const
{
	return (const char **)fAttr;
}


int32
JabberElement::GetAttrCount() const
{
	return fAttrCount;
}


void
JabberElement::Free()
{
	if (fAttrCount != -1) {
		for (int32 i = 0; i < fAttrCount; i++)
			delete [] fAttr[i];
		delete fAttr;
		fAttr = NULL;
		fAttrCount = -1;
	}
}
