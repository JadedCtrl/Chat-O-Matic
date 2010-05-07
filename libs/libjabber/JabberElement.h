/*
 * Copyright 2002, The Olmeki Team.
 * Distributed under the terms of the Olmeki License.
 */
#ifndef _JABBER_ELEMENT_H
#define _JABBER_ELEMENT_H

#include <String.h>

class JabberElement {
public:
					JabberElement();
					~JabberElement();

	void			SetName(const BString& name);
	BString			GetName() const;

	void			SetData(const BString& data);
	BString			GetData() const;

	void			SetAttr(const char** attr);
	const char **	GetAttr() const;

	int32			GetAttrCount() const;

private:
	BString			fName;
	BString			fData;
	char **			fAttr;
	int32			fAttrCount;

	void			Free();
};

#endif	// _JABBER_ELEMENT_H
