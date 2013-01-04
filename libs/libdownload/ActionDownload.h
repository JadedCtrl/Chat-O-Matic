/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _ACTION_DOWNLOAD_H
#define _ACTION_DOWNLOAD_H

#include <curl/curl.h>

#include <Entry.h>
#include <Looper.h>
#include <File.h>
#include <SupportDefs.h>
#include <DownloadManager.h>

#include "Action.h"

class ActionDownload : public Action
{
public:
	enum Status {
		ERROR_OPENFILE,		// 0
		ERROR_CURL_INIT,	// 1
		ERROR_PERFORMING,	// 2
		OK_CONNECTING,		// 3
		OK_PROGRESS,		// 4
		OK_DOWNLOADED		// 5
	};

				ActionDownload(BString URL, entry_ref dest,
	               bool allowResume, BLooper* target = NULL, uint32 msg = 0);
	virtual 	~ActionDownload();

	status_t	Perform(BMessage* errors);
	BString		GetDescription();
	BString		GetLocalPath();

void		
SetLooper(BLooper* loop, uint32 msg) 
{
		fTarget = loop;
		fMsg = msg;
};
	
BLooper*
Looper() 
{
		return fTarget;
}

uint32	
MessageWhat() 
{
		return fMsg;
}

	// For compatibility:
void		
SetExtraData(BString extra) 
{
		extraInfo.AddString("extra", extra);
}

void	
SetKey(BString key, BString data) 
{
		extraInfo.AddString(key.String(), data);
}

void		
SetRef(BString key, entry_ref* ref) 
{
		extraInfo.AddRef(key.String(), ref);
}

status_t
GetRef(BString key, entry_ref* ref) 
{
		return extraInfo.FindRef(key.String(), ref);
}

BString	GetKey(BString key) 
{
		BString data;
		extraInfo.FindString(key.String(), &data);
		return data;
}


void
SetShouldStop(bool stop) 
{
		fShouldStop = stop;
}

bool
ShouldStop() 
{
		return fShouldStop;
}

	void	SetDownloadManager(DownloadManager* downloadManager);

private:
	status_t	openFile(BMessage*);

	static 		void		fillMessage(ActionDownload*, BMessage*);
	static		void		sendProgressX(ActionDownload*, double max, double current);

	//curl callbacks:
	static	size_t		save_file ( void* ptr, size_t size, size_t nmemb, void* stream);
	static	int 		callback (void* clientp, double dltotal, 
							double dlnow, double , double);

	CURL*		curl;
	BString		fUrl;
	entry_ref	fDest;
	BLooper*	fTarget;
	uint32		fMsg;
	BMessage	extraInfo;
	bool		fShouldStop;
	BFile*		file;
	double		resuming;
	bool		fAllowResume;
	DownloadManager* fDownloadManager;

	BString	fFileType;
};

#endif	// _ACTION_DOWNLOAD_H
