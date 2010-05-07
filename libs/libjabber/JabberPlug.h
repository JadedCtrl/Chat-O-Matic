/*
	Interface for a highlivel connection class
	- basic implementations:
		JabberSSLPlug	 (new code used by GoogleTalk)
		JabberSocketPLug (old code BONE/net_server)

	22 sept. 2005 by Andrea Anzani (andrea@tenar.it)
*/
#ifndef JabberPlug_H_
#define JabberPlug_H_
#include <String.h>

class JabberPlug {

	public:
		virtual ~JabberPlug() {};
	//private:
		
		virtual	   int		StartConnection(BString fHost, int32 fPort,void*) = 0;//if >= 0 it's ok.
		virtual	   int		Send(const BString & xml) = 0;	//if >= 0 it's ok.		
		virtual	   int		StopConnection() = 0;
};

#endif
//.
