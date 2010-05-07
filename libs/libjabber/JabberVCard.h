/*
 * Copyright 2009, Pier Luigi Fiorini.
 * Distributed under the terms of the MIT License.
 */
#ifndef _JABBER_VCARD_H
#define _JABBER_VCARD_H

#include <String.h>

class JabberVCard {
public:
				JabberVCard();
				JabberVCard(const JabberVCard& copy);

	void		operator=(const JabberVCard& vcard);

	void 		ParseFrom(const BString& from);

	BString		GetFullName() const;
	BString		GetGivenName() const;
	BString		GetFamilyName() const;
	BString		GetMiddleName() const;
	BString		GetNickname() const;
	BString		GetEmail() const;
	BString		GetURL() const;
	BString		GetBirthday() const;
	BString		GetPhotoMimeType() const;
	BString		GetPhotoContent() const;
	BString		GetPhotoURL() const;
	BString		GetCachedPhotoFile() const;

	BString		GetJid() const;

	void		SetFullName(const BString& firstName);
	void		SetGivenName(const BString& name);
	void		SetFamilyName(const BString& name);
	void		SetMiddleName(const BString& name);
	void		SetNickname(const BString& name);
	void		SetEmail(const BString& email);
	void		SetURL(const BString& url);
	void		SetBirthday(const BString& birthday);
	void		SetPhotoMimeType(const BString& mime);
	void		SetPhotoContent(const BString& content);
	void		SetPhotoURL(const BString& url);
	void		SetCachedPhotoFile(const BString& file);

private:
	BString		fJid;
	BString		fResource;
	BString 	fFullName;
	BString		fGivenName;
	BString		fFamilyName;
	BString		fMiddleName;
	BString		fNickname;
	BString		fEmail;
	BString		fURL;
	BString		fBirthday;
	BString		fPhotoMime;
	BString		fPhotoContent;
	BString		fPhotoURL;
	BString		fCachedPhoto;
};

#endif	// _JABBER_VCARD_H
