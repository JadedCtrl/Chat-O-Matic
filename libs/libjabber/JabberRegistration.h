#ifndef JABBER_REGISTRATION_H
#define JABBER_REGISTRATION_H

#include <String.h>

#include <map>
using std::map;
using std::pair;

class JabberRegistration 
{
public:
							JabberRegistration(const BString & jid = "", bool unRegister = false);
							~JabberRegistration();
							
	typedef map<BString, BString> FieldList;
	typedef pair<BString, BString> FieldPair;
	
	FieldList*				GetFields() const;
	bool 						GetFieldValue(const BString & fieldName, BString & ret);
	BString 				GetJid() const;
	BString 				GetInstructions() const;
	
	bool 					UnRegister() const;
	// Creates a field
	void 					AddField(const BString & field, const BString & value);
	//void 					AddFieldValue(const BString & value);
	// Sets the value of an existant field
	void 					SetFieldValue(const BString & field, const BString & value, bool create);
	
	void 					SetJid(const BString & jid);
	void 					SetInstructions(const BString & instructions);
	
	bool 					HasValues() const;
	
	void					PrintToStream();
	
private:
	bool 					fUnRegister;
	BString 				fJid;
	BString 				fInstructions;
	FieldList * 			fFields;
};


#endif
