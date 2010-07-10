/*
 * Copyright 2010, Andrea Anzani. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Andrea Anzani, andrea.anzani@gmail.com
 */

#include <File.h>
#include <Path.h>
#include <Mime.h>
#include <NodeInfo.h>

#include "ActionDownload.h"
#include "DownloadManager.h"


ActionDownload::ActionDownload(BString URL,
                               entry_ref dest,
                               bool allowResume,
                               BLooper* target,
                               uint32 msg)
	:
	Action(),
	fUrl(URL),
	fDest(dest),
	fTarget(target),
	fMsg(msg),
	fAllowResume(allowResume)
{
	file = NULL;
	curl = NULL;
	resuming = 0.0;
	SetShouldStop(false);
}


ActionDownload::~ActionDownload()
{
}


void
ActionDownload::SetDownloadManager(DownloadManager* downloadManager)
{
	fDownloadManager = downloadManager;
}

status_t
ActionDownload::openFile(BMessage* errors)
{
	uint32 openMode = 0;

	if (fAllowResume)
		openMode = B_WRITE_ONLY | B_CREATE_FILE | B_OPEN_AT_END;
	else
		openMode = B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE;

	if (file) return B_ERROR;

	file = new BFile(&fDest, openMode);

	if (!file || file->InitCheck() != B_OK) {
		BString err("Can't open file!");
		errors->AddInt32("status", ERROR_OPENFILE);
		errors->AddString("error", err);
		return B_ERROR;
	}

	return B_OK;
}


status_t
ActionDownload::Perform(BMessage* errors)
{
	//is it these the right place?
	fillMessage(this, errors);
	//

	CURLcode res;
	status_t	status = B_OK;

	curl = curl_easy_init();

	if (!curl) {
		BString err("Internal error (CURL can't' init)!");
		errors->AddInt32("status", ERROR_CURL_INIT);
		errors->AddString("error", err);
		return B_ERROR;
	}

	curl_easy_setopt(curl, CURLOPT_URL, fUrl.String());
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, callback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this );
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION , 1); //follow the white link!
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER , FALSE); //https
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST , FALSE); //https

	curl_easy_setopt(curl, CURLOPT_FAILONERROR , 1);

	if (fDownloadManager)
		fDownloadManager->FinishCurl(curl);

	//RESUME
	if (fAllowResume) {
		BNode stat(&fDest);
		//  		file->Seek(SEEK_END,0);
		//			size_t resume = (size_t)file->Position();
		off_t size = 0;
		stat.GetSize(&size);

		off_t resume = size;


		if ( resume > 0 ) {
			curl_easy_setopt(curl,	CURLOPT_RESUME_FROM, resume);
			resuming = (double)resume;
		}
	}

	BMessage connect(*errors);
	connect.what = fMsg;
	connect.AddInt32("status", OK_CONNECTING);
	fTarget->PostMessage(&connect);

	res = curl_easy_perform(curl);

	if (res != 0) {
		BString err("Can't connect!");
		errors->AddInt32("status", ERROR_PERFORMING);
		errors->AddString("error", err);
		errors->AddInt32("curl_error", res);
		errors->AddBool("should_stop", ShouldStop());
		status = B_ERROR;
	} else {
		errors->AddInt32("status", OK_DOWNLOADED);
	}

	curl_easy_cleanup(curl);

	errors->AddString("path", GetLocalPath());
	errors->AddRef("entry", &fDest);

	BMimeType mime;

	if (fFileType != "")
		BNodeInfo(file).SetType(fFileType.String());
	else {
		status_t guess = BMimeType::GuessMimeType(&fDest, &mime); // :( return B_ERROR.. why????????????
		if (guess == B_OK)
			BNodeInfo(file).SetType(mime.Type());
	}

	delete file;
	file = NULL;

	if (res == CURLE_BAD_DOWNLOAD_RESUME || res == CURLE_HTTP_RANGE_ERROR) {
		// sarebbe bello chiedere.. va be per ora ci riprova e basta!
		fAllowResume = false;
		errors->MakeEmpty();
		Perform(errors);
	}

	return status;
}

BString
ActionDownload::GetDescription()
{
	return "HTTP File download";
}

BString
ActionDownload::GetLocalPath()
{
	return BPath(&fDest).Path();
}

void
ActionDownload::fillMessage(ActionDownload* ad, BMessage* msg)
{
	//fill with common fileds
	int32 what = msg->what; //preserve original what.
	*msg = ad->extraInfo;

	msg->AddInt64("sid", ad->GetActionID());
	msg->AddString("url", ad->fUrl);

	msg->what = what;
}



void
ActionDownload::sendProgressX(ActionDownload* ad, double max, double current)
{
	BMessage msg(ad->fMsg);

	fillMessage(ad, &msg);

	msg.AddInt32("status", OK_PROGRESS);
	msg.AddFloat("max_progress", (float)max);
	msg.AddFloat("current_progress", (float)current);

	float div = 0.0;
	if (max > 0.0)
		div = ((float)current / (float)max ) * 100;

	msg.AddInt32("percentage_progress", (int32)floor(div));

	double speed = 0;
	if (curl_easy_getinfo(ad->curl, CURLINFO_SPEED_DOWNLOAD, &speed) == CURLE_OK)
		msg.AddFloat("download_speed", (float)speed);


	//TOTALE:
	if (curl_easy_getinfo(ad->curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &speed) == CURLE_OK)
		msg.AddFloat("total_size", (float)speed);

	// ATTUALE: CURLINFO_SIZE_DOWNLOAD
	if (curl_easy_getinfo(ad->curl, CURLINFO_SIZE_DOWNLOAD, &speed) == CURLE_OK)
		msg.AddFloat("actual_size", (float)speed);

	char* array = NULL;


	if (curl_easy_getinfo(ad->curl, CURLINFO_EFFECTIVE_URL, &array) == CURLE_OK)
		msg.AddString("effective_url", BString(array));

	if (ad->fFileType == "" && curl_easy_getinfo(ad->curl, CURLINFO_CONTENT_TYPE, &array) == CURLE_OK) {
		ad->fFileType.SetTo(array);
		BNodeInfo(ad->file).SetType(ad->fFileType.String());
		msg.AddString("filetype", BString(array));
	}

	ad->fTarget->PostMessage(&msg);
}

/*
void
ActionDownload::sendError(BLooper* looper,uint32 amsg,Status st,BString descr){
	BMessage msg(amsg);
	msg.AddInt32("status",st);
	msg.AddString("error",descr);
	looper->PostMessage(&msg);
}
*/
//

int
ActionDownload::callback(void* data,
                         double dltotal,
                         double dlnow,
                         double ,
                         double )
{
	ActionDownload* ad = ((ActionDownload*)data);

	if (ad->fTarget)
		sendProgressX(ad, ad->resuming + dltotal, ad->resuming + dlnow);
	return 0;
}

size_t
ActionDownload::save_file( void* ptr, size_t size, size_t nmemb, void* data)
{
	ActionDownload* ad = ((ActionDownload*)data);

	if (!ad->file) {
		BMessage errors; //FIIX: how to report these errors??
		if (ad->openFile(&errors) != B_OK) {
			printf("ERROR: can't create destination file!\n");
			return 0;
		}
	}
	BFile* file = ad->file;
	if (ad->ShouldStop()) return 0;
	return file->Write(ptr, size * nmemb);
}
