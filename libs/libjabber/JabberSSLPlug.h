/*
	
	JabberSSLPlug (written for GoogleTalk compatibility)

	22 sept. 2005 by Andrea Anzani (andrea@tenar.it)
*/
#ifndef JabberSSLPlug_H_
#define JabberSSLPlug_H_

#include "JabberPlug.h"

#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

#include <OS.h>

#include <Looper.h>
#include <MessageRunner.h>

// public JabberPlug
class JabberSSLPlug : public BLooper, public JabberPlug {

	public:
			JabberSSLPlug(BString forceserver=NULL,int32 port=0);
		   ~JabberSSLPlug();
	//private:
		
			   int		StartConnection(BString fHost, int32 fPort,void* cook);//if >= 0 it's ok.
		static int32 	ReceiveData(void *);    	//thread called function
			   int		Send(const BString & xml);	//if >= 0 it's ok.		
			   int		StopConnection();	
	
			   void		ReceivedData(const char* data, int32);

			   void		MessageReceived(BMessage* );
	private:
	
				   
				BIO* 		bio;
    			SSL_CTX* 	ctx; 
    	
    			BString	ffServer;
    			int32	ffPort;		
				volatile thread_id 			fReceiverThread;
				void*						fCookie; //FIX!
				BMessageRunner*				fKeepAliveRunner;
				int fStartConnectionStatus;					
};

#endif

//--
