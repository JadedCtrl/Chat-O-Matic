#ifndef JABBER_MESSAGE_H
#define JABBER_MESSAGE_H

#include <String.h>

class JabberMessage 
{
public:
						JabberMessage();
						JabberMessage(const JabberMessage &);
						~JabberMessage();
					
	void				operator=(const JabberMessage &);
	
	BString 			GetFrom() const;
	BString 			GetTo() const;
	BString 			GetBody() const;
	BString 			GetStamp() const;
	BString				GetID() const;
	BString				GetType() const  { return fType; }; //by xeD
	BString				GetError() const { return fError; }; //by xeD
	bool					GetOffline() const { return fOffline; }
	BString				GetX(){ return fX;}
	
	void				SetType(const BString & type){ fType=type; } //by xeD;
	void				SetError(const BString & err){ fError=err; } //by xeD;
	void 				SetFrom(const BString & from);
	void 				SetTo(const BString & from);
	void 				SetBody(const BString & body);
	void 				SetStamp(const BString & stamp);
	void				SetID(const BString & id);
	void				SetOffline(const bool b);
	void				SetX(const BString & x){ fX=x; }
	void				PrintToStream();
private:
	BString 			fFrom;
	BString 			fTo;
	BString 			fBody;
	BString 			fStamp;
	BString				fId; 	// by xeD
	BString				fType; 	// by xeD (chat,error,..)
	BString				fError; // error message incluided?
	BString				fX;		// ? FIX!
	bool				fOffline;
};


#endif
