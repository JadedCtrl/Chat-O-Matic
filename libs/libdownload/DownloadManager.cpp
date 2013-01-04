/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <Messenger.h>

#include "DownloadManager.h"
#include "ActionDownload.h"

#define DEFAULT_ITEMS_DOWNLOAD		1

DownloadManager::DownloadManager(BLooper* target)
	:
	fTarget(target)
{
}


void
DownloadManager::Enqueue(QueueType type, ActionDownload* ad)
{
	if (!fQueue[type]) {
		fQueue[type] = new QueueFileDownload("queue_downloads",
			DEFAULT_ITEMS_DOWNLOAD, fTarget, DOWNLOAD_INFO);
	}
	ad->SetDownloadManager(this);
	fQueue[type]->AddAction(ad);
}


void
DownloadManager::TryStopCurrentAction(QueueType type, BString key, BString value)
{

	if (!fQueue[type]) {
		return;
	}

	if (fQueue[type]->Lock()) {
		for (int i = 0; i < fQueue[type]->CountThreads(); i++) {
			ActionDownload* ad = (ActionDownload*)fQueue[type]->CurrentAction(i);
			if (ad &&  ad->GetKey(key).Compare(value.String()) == 0 )
				ad->SetShouldStop(true);
		}

		fQueue[type]->Unlock();
	}
}


bool
DownloadManager::RemoveFromQueue(QueueType type , BString key, BString value)
{

	if (!fQueue[type]) {
		return true;
	}

	if (fQueue[type]->Lock()) {

		for (int32 i = 0; i < fQueue[type]->CountActions(); i++) {
			ActionDownload* ad = (ActionDownload*)fQueue[type]->ActionAt(i);
			if (ad->GetKey(key).Compare(value.String()) == 0 ) {
				fQueue[type]->RemoveActionAt(i);
				fQueue[type]->Unlock();
				return true;
			}
		}

		for (int i = 0; i < fQueue[type]->CountThreads(); i++) {
			ActionDownload* ad = (ActionDownload*)fQueue[type]->CurrentAction(i);

			if (ad &&  ad->GetKey(key).Compare(value.String()) == 0 )
				ad->SetShouldStop(true);
		}
		fQueue[type]->Unlock();
		return false;
	}

	return false;
}


void
DownloadManager::RemoveQueue(QueueType type, BList* removed)
{

	if (!fQueue[type]) {
		return;
	}
	fQueue[type]->Lock();

	while (fQueue[type]->CountActions()) {
		//			ActionDownload *ad = (ActionDownload*)fQueue[type]->ActionAt(0);
		fQueue[type]->RemoveActionAt(0);
	}

	// ***************************************
	//	we did not unlock
	//  so no one can add new item.
	// ***************************************

	for (int i = 0; i < fQueue[type]->CountThreads(); i++) {
		if (fQueue[type]->CurrentAction(i))
			removed->AddItem( (void*)fQueue[type]->CurrentAction(i));
	}
}


void
DownloadManager::LoadProxySetting(BMessage* data)
{

	bool value;

	fEnabled = false;
	fAddress.SetTo("");
	fUserpwd.SetTo("");
	fPort = 0;

	if (data->FindBool("enable", &value) == B_OK)
		fEnabled = value;

	if (!fEnabled)
		return;

	BString username, password;
	data->FindString("username", &username);
	data->FindInt32("port", &fPort);
	data->FindString("address", &fAddress);
	data->FindString("password", &password);
	if (username != "" || password != "")
		fUserpwd << username << ":" << password;
}


void
DownloadManager::LoadDownloadSetting(BMessage* data)
{

	//FIXME: fQueue[XXX] can be null.

	//here magic stuff! :=)
	int32 number = 1;
	if (data->FindInt32("max_downloads", &number) == B_OK) {
		fQueue[DOWNLOADS_QUEUE]->SetDownloadCount(number);
	}
}


void
DownloadManager::FinishCurl(CURL* curl)
{
	if (!fEnabled)
		return;

	curl_easy_setopt(curl, CURLOPT_PROXY, fAddress.String());
	curl_easy_setopt(curl, CURLOPT_PROXYPORT, fPort);
	curl_easy_setopt(curl, CURLOPT_PROXYTYPE , CURLPROXY_HTTP);

	if (fUserpwd != "")
		curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD , fUserpwd.String() );
}


thread_id
DownloadManager::SingleThreadAction(ActionDownload* action)
{
	action->SetDownloadManager(this);
	if (action->Looper() == NULL) {
		action->SetLooper(fTarget, DOWNLOAD_INFO);
	}
	thread_id id = spawn_thread(DownloadManager::SingleThreadPerform, 
		"single_action_thread", B_NORMAL_PRIORITY, (void*)action);
	resume_thread(id);
	return id;
}


int32
DownloadManager::SingleThreadPerform(void* a)
{

	ActionDownload*	ad = (ActionDownload*)a;

	// perform the action
	BMessage err;
	//status_t status =
	ad->Perform(&err);
	// do post-perform!
	if (ad->Looper()) {
		err.what = ad->MessageWhat();
		BMessenger(ad->Looper()).SendMessage(&err);
	}
	return 0;
}

