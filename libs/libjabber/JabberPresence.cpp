#include "JabberPresence.h"
#include "States.h"
#include "Logger.h"

JabberPresence::JabberPresence() 
{
	fStatus = "";
	fJid = "";
	fType = "";
	fShow = S_OFFLINE;
	fResource = "";
}
void
JabberPresence::PrintToStream() 
{
	logmsg("\nJabberPresence");
	logmsg("   Status:  %s",fStatus.String());
	logmsg("    Show:  %ld",fShow);
	logmsg("      Jid:  %s",fJid.String());
	
}
JabberPresence::JabberPresence(const JabberPresence & copy)
{
	SetStatus(copy.GetStatus());
	SetJid(copy.GetJid());
	SetType(copy.GetType());
	SetResource(copy.GetResource());
	fShow = copy.GetShow();
}

JabberPresence::~JabberPresence() 
{
}

void
JabberPresence::operator=(const JabberPresence & rhs)
{
	if (this == &rhs)
		return;
	
	SetStatus(rhs.GetStatus());
	SetJid(rhs.GetJid());
	SetType(rhs.GetType());
	SetResource(rhs.GetResource());
	fShow = rhs.fShow;
}

int32
JabberPresence::GetShow() const
{
	return fShow;
}

BString
JabberPresence::GetType() const
{
	return fType;
}

BString
JabberPresence::GetStatus() const
{
	return fStatus;
}

BString
JabberPresence::GetJid() const
{
	return fJid;
}

BString
JabberPresence::GetResource() const
{
	return fResource;
}

void
JabberPresence::SetShowFromString(const BString & show) 
{
	if (show != "")
	{
		if (!show.ICompare("xa"))
			fShow = S_XA;
		else if (!show.ICompare("away"))
			fShow = S_AWAY;
		else if (!show.ICompare("dnd"))
			fShow = S_DND;
		else if (!show.ICompare("chat"))
			fShow = S_CHAT;
	}
}

void
JabberPresence::SetShow(int32 show) 
{
	switch(show) 
	{
		case S_XA:
			fShow = S_XA;
			break;
		case S_AWAY:
			fShow = S_AWAY;
			break;
		case S_ONLINE:
			fShow = S_ONLINE;
			break;
		default:
			fShow = S_OFFLINE;
	}
}

void
JabberPresence::SetType(const BString & type) 
{
	fType = type;
	if(fType.ICompare("unavailable") == 0)
		SetShow(S_OFFLINE);
}

void
JabberPresence::SetStatus(const BString & status) 
{
	fStatus = status;
}

void
JabberPresence::SetJid(const BString & jid) 
{
	fJid = jid;
}

void
JabberPresence::SetResource(const BString & resource)
{
	fResource = resource;
}

void
JabberPresence::ParseFrom(const BString & from)
{
	fJid = "";
	fResource = "";
		
	int32 i = from.FindFirst('/');
	if (i != -1) 
	{
		from.CopyInto(fJid, 0, i);
		from.CopyInto(fResource, i + 1, from.Length());
	} 
	else 
	{
		fJid = from;
	}
}

BString
JabberPresence::GetPhotoSHA1() const
{
	return fPhotoSHA1;
}

BString
JabberPresence::GetPhotoPath() const
{
	return fPhotoPath;
}
		
void
JabberPresence::SetPhotoSHA1(const BString & sha1)
{
		fPhotoSHA1 = sha1;
}

void
JabberPresence::SetPhotoPath(const BString & path)
{
		fPhotoPath = path;
}
