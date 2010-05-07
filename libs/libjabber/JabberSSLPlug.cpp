#include "JabberSSLPlug.h"
#include "JabberHandler.h"
#include "Logger.h"

#define msnmsgPing 'ping'

JabberSSLPlug::JabberSSLPlug(BString forceserver, int32 port){

	bio = NULL;
	ctx = NULL;
	
	ffServer = forceserver;
	ffPort = port;

	Run();
	
	fKeepAliveRunner = new BMessageRunner(BMessenger(NULL, (BLooper *)this),
		new BMessage(msnmsgPing), 60000000, -1);

	/* Set up the library */
	SSL_library_init();
    ERR_load_BIO_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
}


JabberSSLPlug::~JabberSSLPlug(){

		if ( fKeepAliveRunner)
			delete fKeepAliveRunner;
		if(bio != NULL && ctx !=NULL) StopConnection();
}

void
JabberSSLPlug::MessageReceived(BMessage* msg){

		if(msg->what == msnmsgPing){
			Send(" ");
		}
		else
			BLooper::MessageReceived(msg);

}
int
JabberSSLPlug::StartConnection(BString fServer, int32 fPort,void* cookie){
	
	StopConnection();
	
	BString fHost;
	
	if(ffServer!="")	
		fHost << ffServer << ":" << ffPort;
	else
		fHost << fServer << ":" << fPort;
	
	logmsg("StartConnection to %s",fHost.String());

	SSL * ssl;
	int result = 0;

    /* Set up the SSL context */

     ctx = SSL_CTX_new(SSLv23_client_method());

	 bio = BIO_new_ssl_connect(ctx);

    /* Set the SSL_MODE_AUTO_RETRY flag */

    BIO_get_ssl(bio, & ssl);
    SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

    /* Create and setup the connection */	
	
    
    BIO_set_conn_hostname(bio, fHost.String());

    if(BIO_do_connect(bio) <= 0)
    {
        logmsg("Error attempting to connect");
        ERR_print_errors_fp(stderr);
        BIO_free_all(bio);
        SSL_CTX_free(ctx);
        bio = NULL;
        ctx = NULL;
        result = -1;
    }
    
	if (result != -1)
	{
		fCookie = cookie;

		fReceiverThread = spawn_thread (ReceiveData, "opensll receiver", B_LOW_PRIORITY, this); 
		
		if (fReceiverThread != B_ERROR) 
	   		resume_thread(fReceiverThread); 
		else 
		{
			logmsg("failed to resume the thread!");
			ERR_print_errors_fp(stderr);
       		BIO_free_all(bio);
       		SSL_CTX_free(ctx);
       		bio = NULL;
       		ctx = NULL;
			result = -1;
		}
	}
	
	logmsg("DONE: StartConnection to %s",fHost.String());
	fStartConnectionStatus = result;
	return result;	
}

/* static function called by the therad.*/
int32
JabberSSLPlug::ReceiveData(void * pHandler){
	
	char data[1024];
	int length = 0;
	JabberSSLPlug * plug = reinterpret_cast<JabberSSLPlug * >(pHandler);
	
	while (true) 
	{
				
		if ((length = (int)BIO_read(plug->bio, data, 1023) ) > 0) 
		{
			data[length] = 0;
			logmsg("SSLPlug<<\n%s", data);
		} 
		else 
		{
			if(!BIO_should_retry(plug->bio)) 
			{			
				//uhm really and error!
				logmsg("SSLPlug ERROR READING! (maybe dropped connection?)");
				ERR_print_errors(plug->bio);
				plug->ReceivedData(NULL, 0);
				return 0;			
			}
		}
		plug->ReceivedData(data,length);
	}	
	return 0;
}

void
JabberSSLPlug::ReceivedData(const char* data, int32 len){
	JabberHandler * handler = reinterpret_cast<JabberHandler * >(fCookie);
	if(handler)
		handler->ReceivedData(data,len);
}


int
JabberSSLPlug::Send(const BString & xml)
{
	if (fStartConnectionStatus == -1)
		return 0;
		
	logmsg("SSLPlug>>\n%s", xml.String());
	return BIO_write(bio, xml.String(), xml.Length());
}

int		
JabberSSLPlug::StopConnection()
{
	if(fReceiverThread)
	{
		//Thread Killing!
		suspend_thread(fReceiverThread);
		kill_thread(fReceiverThread);
	}
	
	fReceiverThread=0;
	fStartConnectionStatus = 0;
	
	BIO_free_all(bio);
    SSL_CTX_free(ctx);

	bio = NULL;
	ctx = NULL;

	return 0;
}


//--

