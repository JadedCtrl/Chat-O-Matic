
#include <stdio.h>
#include <Application.h>
#include "DownloadManager.h"
#include "ActionDownload.h"
#include <Entry.h>
#include "PercentageWindow.h"
#include <TranslationUtils.h>

class TestApp : public BApplication
{
	public:
		TestApp():BApplication("application/xeD.libdownload.test"){};
		
		void ReadyToRun()
		{
		
			printf("Ready To Run\n");
			
			entry_ref ref;
			get_ref_for_path("test.xxx", &ref);
			ActionDownload* ad = new ActionDownload("http://www.webelin.it/website/wp-content/uploads/pdf/newsletter-webelin.pdf", ref, false);
			
			BBitmap* icon = BTranslationUtils::GetBitmap("/boot/home/image.bmp");
			percWindow = new PercentageWindow("Download test","Downloading useless test stuff...", icon);
			percWindow->Show();
			
			/*BMessage msg;
			msg.AddBool("enable", true);
			msg.AddString("username", "");
			msg.AddString("address", "");
			msg.AddString("password", "");
			msg.AddInt32("port", 8080);
			*/
			fDownloadManager = new DownloadManager(this);
			//fDownloadManager->LoadProxySetting(&msg);
			fDownloadManager->SingleThreadAction(ad);
			percWindow->SetPercentage(0);

			
		}
		
		void MessageReceived(BMessage* msg)
		{
			if (msg->what == DOWNLOAD_INFO)
			{
				int32 status = msg->FindInt32("status");
				switch (status)
				{
					case ActionDownload::ERROR_OPENFILE:
						printf("Error openig dest file\n");
						break;
					case ActionDownload::ERROR_CURL_INIT:
						printf("Error while init curl\n");
						break;
					case ActionDownload::ERROR_PERFORMING:
						printf("Error performing download\n");
						msg->PrintToStream();
						break;
					case ActionDownload::OK_CONNECTING:
						printf("Connecting\n");
						break;
					case ActionDownload::OK_PROGRESS:
						printf("Progress\n");
						msg->PrintToStream();
						percWindow->SetPercentage(msg->FindInt32("percentage_progress"));
						break;
					case ActionDownload::OK_DOWNLOADED:
						printf("Downloaded\n");
						break;					 
					default:
						msg->PrintToStream();
						break;
				};											
			}
			else
				BApplication::MessageReceived(msg);

		}
		
	private:
		DownloadManager*	fDownloadManager;
		PercentageWindow* 	percWindow;
		
};

int main()
{
	TestApp tApp;
	tApp.Run();
	return 0;
}
