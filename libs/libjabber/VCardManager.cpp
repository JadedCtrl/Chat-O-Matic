#include "VCardManager.h"
#include <File.h>
#include <Entry.h>
#include <FindDirectory.h>
#include "JabberPresence.h"
#include "JabberContact.h"
#include "JabberVCard.h"
#include "JabberHandler.h"
#include "Logger.h"
#include <stdio.h>
#include "SHA1.h"

VCardManager::VCardManager(JabberHandler* jabberHandler) : fJabberHandler(jabberHandler)
{
	BPath fCacheFolderPath;
	find_directory(B_USER_SETTINGS_DIRECTORY, &fCacheFolderPath);
	fCacheFolderPath.Append("libjabber");
	fCacheFolder.SetTo(fCacheFolderPath.Path());
	
	fCacheFolder.CreateDirectory(fCacheFolderPath.Path(), &fCacheFolder);
	
	fCachePath = fCacheFolderPath;
	fCachePath.Append("jabber-roster-cache");

	BFile	*file = new BFile(fCachePath.Path(), B_READ_ONLY);
	fCache.Unflatten(file);
	delete file;
}

void	
VCardManager::SaveCache()
{
	BFile	*file = new BFile(fCachePath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	fCache.Flatten(file);
	delete file;
}

void	
VCardManager::VCardReceived(JabberContact* contact)
{
	logmsg("VCardReceived:  for %s\n",  contact->GetJid().String());
	if (contact->GetVCard()->GetPhotoContent() == "")
		return;
		
	BMessage jid;
	if (fCache.FindMessage(contact->GetJid().String(), &jid) != B_OK)
	{
		fCache.AddMessage(contact->GetJid().String(), &jid);
		logmsg("no vCard request in cache! adding..\n");
		SaveCache();
	}

	BString sha1;
	if (jid.FindString("photo-sha1", &sha1) != B_OK || sha1 == "" )
	{
		// let's try to make an sha1..
		CSHA1 s1;
		char hash[256];
		s1.Reset();	
		s1.Update((unsigned char*)contact->GetVCard()->GetPhotoContent().String(), contact->GetVCard()->GetPhotoContent().Length());
		s1.Final();
		s1.ReportHash(hash, CSHA1::REPORT_HEX);
		sha1.SetTo(hash, 256);
		logmsg("sha1 created: %s for %s adding to cache..\n", sha1.String(), contact->GetJid().String());
		jid.AddString("photo-sha1", sha1.String());
		fCache.ReplaceMessage(contact->GetJid().String(), &jid);
		SaveCache();
	}
		
	//save to file.
	BPath newFile(&fCacheFolder); 
	newFile.Append(sha1.String());
	
	BFile file(newFile.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	file.Write(contact->GetVCard()->GetPhotoContent().String(), contact->GetVCard()->GetPhotoContent().Length());
	contact->GetVCard()->SetCachedPhotoFile(newFile.Path());
	if (contact->GetJid() != fJabberHandler->GetJid())
		fJabberHandler->GotBuddyPhoto(contact->GetJid(), newFile.Path());
}
		
void
VCardManager::RefinePresence(JabberPresence* presence)
{
	logmsg("RefinePresence: [%s] for %s\n", presence->GetPhotoSHA1().String(), presence->GetJid().String());
	BMessage jid;
	if (fCache.FindMessage(presence->GetJid().String(), &jid) != B_OK)
	{
		logmsg("   not found in cache.. adding\n");
		jid.AddString("photo-sha1", presence->GetPhotoSHA1().String());
		fCache.AddMessage(presence->GetJid().String(), &jid);
		SaveCache();
		logmsg("...asking for downloading the image..\n");
		fJabberHandler->RequestVCard(presence->GetJid());
	}
	else
	{
		logmsg("..found in cache!\n");
		BString sha1;
		if ( jid.FindString("photo-sha1", &sha1) == B_OK )
		{
			if (sha1.ICompare(presence->GetPhotoSHA1()) != 0)
			{
				logmsg("..existing sha1 is different, asking new vcard..\n");
				jid.ReplaceString("photo-sha1", presence->GetPhotoSHA1().String());
				SaveCache();
				fJabberHandler->RequestVCard(presence->GetJid());
			}
			else
			{
				if (sha1 == "")
				{
					fJabberHandler->GotBuddyPhoto(presence->GetJid(), "");
				}
				else
				{
					BPath newFile(&fCacheFolder); 
					newFile.Append(sha1.String());
					logmsg("..sha1 match.. checking if file exits..(%s)\n", newFile.Path());
					if(BEntry(newFile.Path()).Exists())
					{
						logmsg(".. yes it exists!\n");
						fJabberHandler->GotBuddyPhoto(presence->GetJid(), newFile.Path());
					}
					else
					{
						logmsg("..no it doesn't, asking new vcard..\n");
						fJabberHandler->RequestVCard(presence->GetJid());
					}
				}
			}
		}
		else
		{
			fJabberHandler->RequestVCard(presence->GetJid());
		}	
	}

}
