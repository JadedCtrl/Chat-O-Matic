#include "JabberSocketPlug.h"
#include "Logger.h"

#ifdef NETSERVER_BUILD 
#	include <netdb.h> 
#	include <sys/socket.h> 
#	include <Locker.h>
#endif 

#ifdef BONE_BUILD
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netdb.h>
#  include <unistd.h>
#endif 

#ifdef __HAIKU__
#	include <arpa/inet.h>
#	include <sys/socket.h>
#	include <netdb.h>
#endif

#include "JabberHandler.h"

JabberSocketPlug::JabberSocketPlug(){
	
	fReceiverThread = -1;
	fSocket = -1;
	
	#ifdef NETSERVER_BUILD
	fEndpointLock = new BLocker();
	#endif
}

JabberSocketPlug::~JabberSocketPlug(){
}

int
JabberSocketPlug::StartConnection(BString fHost, int32 fPort,void* cookie){
	
	logmsg("StartConnection to %s:%ld",fHost.String(),fPort);
	struct sockaddr_in remoteAddr;
	remoteAddr.sin_family = AF_INET;

				
#ifdef BONE_BUILD
    if (inet_aton(fHost.String(), &remoteAddr.sin_addr) == 0)
#elif NETSERVER_BUILD 
    if ((int)(remoteAddr.sin_addr.s_addr = inet_addr (fHost.String())) <= 0) 
#endif
	{
       	struct hostent * remoteInet(gethostbyname(fHost.String()));
       	if (remoteInet)
       		remoteAddr.sin_addr = *((in_addr *)remoteInet->h_addr_list[0]);
       	else 
       	{
			logmsg("failed (remoteInet) [%s]",fHost.String());
		}
    }

    remoteAddr.sin_port = htons(fPort);
    
    if ((fSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    {
    	logmsg("failed to create socket");
    	fSocket = -1;
    }
    
    if (connect(fSocket, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) < 0) 
    {
	   	logmsg("failed to connect socket");
    	fSocket = -1;
    }
   
		
	fCookie = cookie;

	fReceiverThread = spawn_thread (ReceiveData, "socket receiver", B_LOW_PRIORITY, this); 
		
	if (fReceiverThread != B_ERROR) 
	   	resume_thread(fReceiverThread); 
	else 
	{
		logmsg("failed to resume the thread!");
		return -1;
	}
	
	logmsg("DONE: StartConnection to %s:%ld",fHost.String(),fPort);
	return fSocket;	
}


int32
JabberSocketPlug::ReceiveData(void * pHandler){
	
	char data[1024];
	int length = 0;
	JabberSocketPlug * plug = reinterpret_cast<JabberSocketPlug * >(pHandler);
	
	while (true) 
	{
		#ifdef NETSERVER_BUILD 
			plug->fEndpointLock->Lock(); 
		#endif
		
		if ((length = (int)recv(plug->fSocket, data, 1023, 0)) > 0) 
		{
			#ifdef NETSERVER_BUILD 
				plug->fEndpointLock->Unlock(); 
			#endif
			data[length] = 0;
			logmsg("SocketPlug<<\n%s", data);
		} 
		else 
		{
			#ifdef NETSERVER_BUILD 
				plug->fEndpointLock->Unlock(); 
			#endif
			
			plug->ReceivedData(NULL,0);
			return 0;
		}
		
		
		plug->ReceivedData(data,length);
	}

	return 0;
}

void
JabberSocketPlug::ReceivedData(const char* data,int32 len){
	JabberHandler * handler = reinterpret_cast<JabberHandler * >(fCookie);
	if(handler)
		handler->ReceivedData(data,len);
}

int
JabberSocketPlug::Send(const BString & xml){
	
	if (fSocket) 
	{
		#ifdef NETSERVER	 
			fEndpointLock->Lock(); 
		#endif	
		
		logmsg("SocketPlug>>\n%s", xml.String());
		
		if(send(fSocket, xml.String(), xml.Length(), 0) == -1)
			return -1;
		
		#ifdef NETSERVER_BUILD 
			fEndpointLock->Unlock(); 
		#endif
	} 
	
	else
	
	{
		logmsg("Socket not initialized");
		return -1;
	}
	return 0;
}

int		
JabberSocketPlug::StopConnection(){
	
	//Thread Killing!
	suspend_thread(fReceiverThread);
	
	if(fReceiverThread)	kill_thread(fReceiverThread);
	
	fReceiverThread=0;
	
	#ifdef BONE_BUILD 
		close(fSocket);
	#elif NETSERVER_BUILD 
		closesocket(fSocket); 
	#endif
	
	return 0;
}


//--

