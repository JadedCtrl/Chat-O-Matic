#include "JabberRegistration.h"
#include "Logger.h"

JabberRegistration::JabberRegistration(const BString & jid, bool unRegister)
{
	fJid = jid;
	fUnRegister = unRegister;
	fInstructions = "";
	fFields = new FieldList;
}

void
JabberRegistration::PrintToStream() 
{
	logmsg(" ** JabberRegistration **");
	logmsg("    fJid:  %s",fJid.String());
	logmsg("  fInstructions:  %s",fInstructions.String());
	logmsg(" ** JabberRegistrationFields **");
	
}
JabberRegistration::~JabberRegistration()
{
	delete fFields;
}

void
JabberRegistration::SetJid(const BString & jid)
{
	fJid = jid;
}

void
JabberRegistration::SetInstructions(const BString & instructions)
{
	fInstructions = instructions;
}

void
JabberRegistration::AddField(const BString & field, const BString & value)
{
	FieldPair p;
	p.first = field;
	p.second = value;
	fFields->insert(p);
	
	logmsg("Field added %s %s",field.String(),value.String());
}

void
JabberRegistration::SetFieldValue(const BString & field, const BString & value, bool create)
{
	FieldList::iterator i = fFields->find(field);
	if (i == fFields->end())	// not found?
	{
		if (create)
			AddField(field, value);
		return;
	}
	FieldPair pair = *i;
	fFields->erase(i);
	pair.second = value;
	fFields->insert(pair);
}

bool
JabberRegistration::GetFieldValue(const BString & fieldName, BString & ret)
{
	FieldList::iterator i = fFields->find(fieldName);
	if (i != fFields->end())
	{
		ret = (*i).second;
		return true;
	}
	
	return false;
}

BString
JabberRegistration::GetJid() const
{
	return fJid;
}

bool
JabberRegistration::UnRegister() const
{
	return fUnRegister;
}	

JabberRegistration::FieldList *
JabberRegistration::GetFields() const
{
	return fFields;
}

BString
JabberRegistration::GetInstructions() const
{
	return fInstructions;
}

bool
JabberRegistration::HasValues() const
{
	if (fFields->size() > 0)
	{
		if ((*(fFields->begin())).second != "")
			return true;
	}
	return false;
}
