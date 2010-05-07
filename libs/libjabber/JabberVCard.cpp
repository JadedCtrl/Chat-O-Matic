/*
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */

#include "JabberVCard.h"
#include "Base64.h"


JabberVCard::JabberVCard()
{
}


JabberVCard::JabberVCard(const JabberVCard& copy)
{
	SetFullName(copy.GetFullName());
	SetGivenName(copy.GetGivenName());
	SetFamilyName(copy.GetFamilyName());
	SetMiddleName(copy.GetMiddleName());
	SetNickname(copy.GetNickname());
	SetEmail(copy.GetEmail());
}


void
JabberVCard::operator=(const JabberVCard& vcard)
{
	if (this == &vcard)
		return;

	SetFullName(vcard.GetFullName());
	SetGivenName(vcard.GetGivenName());
	SetFamilyName(vcard.GetFamilyName());
	SetMiddleName(vcard.GetMiddleName());
	SetNickname(vcard.GetNickname());
	SetEmail(vcard.GetEmail());
}


BString
JabberVCard::GetJid() const
{
	return fJid;
}



void
JabberVCard::ParseFrom(const BString& from)
{
	fJid = "";
	fResource = "";

	int32 i = from.FindFirst('/');
	if (i != -1) {
		from.CopyInto(fJid, 0, i);
		from.CopyInto(fResource, i + 1, from.Length());
	} else
		fJid = from;
}


BString
JabberVCard::GetFullName() const
{
	return fFullName;
}


BString
JabberVCard::GetGivenName() const
{
	return fGivenName;
}


BString
JabberVCard::GetFamilyName() const
{
	return fFamilyName;
}


BString
JabberVCard::GetMiddleName() const
{
	return fMiddleName;
}


BString
JabberVCard::GetNickname() const
{
	return fNickname;
}


BString
JabberVCard::GetEmail() const
{
	return fEmail;
}


BString
JabberVCard::GetURL() const
{
	return fURL;
}


BString
JabberVCard::GetBirthday() const
{
	return fBirthday;
}


BString
JabberVCard::GetPhotoMimeType() const
{
	return fPhotoMime;
}


BString
JabberVCard::GetPhotoContent() const
{
	return fPhotoContent;
}


BString
JabberVCard::GetPhotoURL() const
{
	return fPhotoURL;
}


BString
JabberVCard::GetCachedPhotoFile() const
{
	return fCachedPhoto;
}


void
JabberVCard::SetFullName(const BString& firstName)
{
	fFullName = firstName;
}


void
JabberVCard::SetGivenName(const BString& name)
{
	fGivenName = name;
}


void
JabberVCard::SetFamilyName(const BString& name)
{
	fFamilyName = name;
}


void
JabberVCard::SetMiddleName(const BString& name)
{
	fMiddleName = name;
}


void
JabberVCard::SetNickname(const BString& name)
{
	fNickname = name;
}


void
JabberVCard::SetEmail(const BString& email)
{
	fEmail = email;
}


void
JabberVCard::SetURL(const BString& url)
{
	fURL = url;
}


void
JabberVCard::SetBirthday(const BString& birthday)
{
	fBirthday = birthday;
}


void
JabberVCard::SetPhotoMimeType(const BString& mime)
{
	fPhotoMime = mime;

	// We either get base64 encoded data or grab image
	// from an URL
	fPhotoURL = "";
}


void
JabberVCard::SetPhotoContent(const BString& content)
{
	// Decode base64
	fPhotoContent = Base64::Decode(content);

	// We either get base64 encoded data or grab image
	// from an URL
	fPhotoURL = "";
}


void
JabberVCard::SetPhotoURL(const BString& url)
{
	fPhotoURL = url;

	// If we previously set the MIME type and/or content
	// we must remove them because we either fetch
	// the photo from a URL or get it from base64
	// encoded data
	fPhotoMime = "";
	fPhotoContent = "";
}


void
JabberVCard::SetCachedPhotoFile(const BString& file)
{
	fCachedPhoto = file;
}
