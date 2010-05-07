#ifndef JABBER_PRESENCE_H
#define JABBER_PRESENCE_H

#include <String.h>

class JabberPresence 
{
public:
						JabberPresence();
						// Copy constructor
						JabberPresence(const JabberPresence &);
						~JabberPresence();
	
	void				operator=(const JabberPresence & rhs);
	
	int32 				GetShow() const;
	BString 			GetType() const;
	BString				GetStatus() const;
	BString				GetJid() const;
	BString				GetResource() const;
	
	BString				GetPhotoSHA1() const;
	BString				GetPhotoPath() const;
		
	void				SetPhotoSHA1(const BString & sha1);
	void				SetPhotoPath(const BString & path);
	
	void 				SetShowFromString(const BString & show);
	void 				SetShow(int32 show);		
	void 				SetType(const BString & type);
	void 				SetStatus(const BString & status);
	void 				SetJid(const BString & jid);
	void 				SetResource(const BString & resource);
	
	void 				ParseFrom(const BString & from);
	void				PrintToStream();
private:
	BString 			fStatus;
	BString 			fJid;
	BString 			fType;
	int32 				fShow;
	BString 			fResource;
	BString				fPhotoSHA1;
	BString				fPhotoPath;
};

#endif	// JABBER_PRESENCE_H
