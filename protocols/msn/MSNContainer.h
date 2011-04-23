/*
 * Copyright 2011 Barrett
 * All rights reserved. Distributed under the terms of the GPL license.
 * 
 * This is a simple class used as container for various informations, like
 * avatar msn object, it is used essentially when creating a new switchboard connection.
 *
 */

#ifndef MSNCONTAINER_H
#define MSNCONTAINER_H

using namespace std;

#include <SupportDefs.h>
#include <string>

class MSNContainer {
public:
			MSNContainer(string buddy);
			MSNContainer(string msg, string buddy);
	virtual ~MSNContainer();
	// if it is also a message
	bool	IsMessage() { return fIfMsg; }
	// if it is used for advanced features like avatars
	bool	HasObject() { if (fMSNObject == "") return false; else return true; }

	//void	SetMessage(const char* msg) { fMessage = msg; }
	string	Message() { return fMessage; }
	//void	SetRCPT(const char* rcpt) { fRCPT = rcpt; }
	string	Buddy() { return fRCPT; }
	void	SetObject(string msnobj) { fMSNObject = msnobj; }
	string	Object() { return fMSNObject; }
private:
	string	fRCPT;
	string	fMessage;
	string	fMSNObject;

	bool	fIfMsg;
	
};

#endif // MSNCONTAINER_H
