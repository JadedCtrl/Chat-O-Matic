/*
	
	JabberSocketPlug (old code BONE/net_server)

	22 sept. 2005 by Andrea Anzani (andrea@tenar.it)
*/
#ifndef JabberSocketPlug_H_
#define JabberSocketPlug_H_

#include "JabberPlug.h"
#include <Locker.h>

class JabberSocketPlug : public JabberPlug {

	public:
			JabberSocketPlug();
		    virtual ~JabberSocketPlug();
	//private:
		
			   int		StartConnection(BString fHost, int32 fPort,void* cook);//if >= 0 it's ok.
		static int32 	ReceiveData(void *);    	//thread called function
			   int		Send(const BString & xml);	//if >= 0 it's ok.		
			   int		StopConnection();	
	
			   void		ReceivedData(const char* data,int32);
	private:
	
	
		int32 						fSocket;
		volatile thread_id 			fReceiverThread;
		void*						fCookie; //FIX!
							
	#ifdef NETSERVER_BUILD
		BLocker * 					fEndpointLock;
	#endif				

};

#endif

//--
