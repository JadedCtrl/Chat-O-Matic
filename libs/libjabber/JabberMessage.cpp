#include "JabberMessage.h"
#include "Logger.h"

JabberMessage::JabberMessage() 
{
	fTo = "";
	fFrom = "";
	fBody = "";
	fStamp = "";
	fId = "";
	fOffline= false;
	fType="";
	fError="";
	fX="";
}

JabberMessage::JabberMessage(const JabberMessage & copy)
{
	fTo = copy.fTo;
	fFrom = copy.fFrom;
	fBody = copy.fBody;
	fStamp = copy.fStamp;
	fOffline= copy.fOffline;
	fId = copy.fId;
	fType=copy.fType;
	fError=copy.fError;
	fX=copy.fX;
}

void
JabberMessage::PrintToStream() 
{
	logmsg(" ** JabberMessage **");
	logmsg("    To:  %s",fTo.String());
	logmsg("    Id:  %s",fId.String());
	logmsg("  From:  %s",fFrom.String());
	logmsg("  Body:  %s",fBody.String());
	logmsg(" Stamp:  %s",fStamp.String());
	logmsg("  Type:  %s",fType.String());
	logmsg(" Error:  %s",fError.String());
	logmsg("     X:  %s",fX.String());
}


JabberMessage::~JabberMessage() 
{
}

void
JabberMessage::operator=(const JabberMessage & copy)
{
	fTo = copy.fTo;
	fFrom = copy.fFrom;
	fBody = copy.fBody;
	fStamp = copy.fStamp;
	fOffline= copy.fOffline;
	fId = copy.fId;
	fType=copy.fType;
	fError=copy.fError;
	fX=copy.fX;
}

BString 
JabberMessage::GetFrom() const
{
	return fFrom;	
}

BString
JabberMessage::GetTo() const
{
	return fTo;	
}

BString
JabberMessage::GetBody() const
{
	return fBody;	
}

BString
JabberMessage::GetStamp() const
{
	return fStamp;
}

BString
JabberMessage::GetID() const
{
	return fId;
}
void
JabberMessage::SetFrom(const BString & from)
{
	fFrom = from;
}

void
JabberMessage::SetTo(const BString & to)
{
	fTo = to;
}

void
JabberMessage::SetBody(const BString & body)
{
	fBody = body;
}

void
JabberMessage::SetStamp(const BString & stamp)
{
	fStamp = stamp;
}
void
JabberMessage::SetID(const BString & id)
{
	fId = id;
}
void
JabberMessage::SetOffline(const bool b)
{
 	fOffline = b;
}
