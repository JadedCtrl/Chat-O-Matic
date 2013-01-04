/*
 * Copyright 2006-2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 */
#ifndef _DOWNLOAD_MANAGER_H
#define _DOWNLOAD_MANAGER_H

#include <Looper.h>
#include <List.h>
#include <SupportDefs.h>
#include <curl/curl.h>

#include "QueueFileDownload.h"

#define		DOWNLOAD_INFO	'dwni'

class ActionDownload;

class DownloadManager
{
public:
	enum 		QueueType {DOWNLOADS_QUEUE  = 0};

				DownloadManager(BLooper* target);

	void		Enqueue(QueueType, ActionDownload*);

	void		TryStopCurrentAction(QueueType, BString key, BString value);

	bool		RemoveFromQueue(QueueType, BString key, BString value);

	void 		RemoveQueue(QueueType, BList* removed);

	void		LoadProxySetting(BMessage* data);
	void		LoadDownloadSetting(BMessage* data);

	thread_id	SingleThreadAction(ActionDownload* action);

	void		FinishCurl(CURL* curl);

private:
	static int32		SingleThreadPerform(void* a);

	QueueFileDownload*	fQueue[3];

	// Proxy
	int32				fPort;
	BString				fAddress;
	BString				fUserpwd;
	bool				fEnabled;
	BLooper*			fTarget;
};

#endif	// _DOWNLOAD_MANAGER_H
