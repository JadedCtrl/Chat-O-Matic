/* i don't like this class name */

#ifndef Emoconfig_h_
#define Emoconfig_h_

#include <Message.h>
#include <expat.h>

class Emoconfig : public BMessage
{
public:
	Emoconfig(const char* xmlfile);
	~Emoconfig();
	int numfaces;
	BMessage					menu;

	float	GetEmoticonSize() {
		return fEmoticonSize;
	}

private:

	float		fEmoticonSize;
	XML_Parser	fParser;

	static void StartElement(void* pUserData, const char* pName, const char** pAttr);
	static void EndElement(void* pUserData, const char* pName);
	static void Characters(void* pUserData, const char* pString, int pLen);

};

#endif

