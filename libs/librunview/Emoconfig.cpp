#include "Emoconfig.h"

#include <File.h>
#include <stdio.h>
#include <stdlib.h>
#include <Bitmap.h>
#include <String.h>
#include <Path.h>
#include <TranslationUtils.h>
#include "SmileTextRender.h"

//tmp
BMessage*	faces = NULL;
bool 	valid = false;
bool 	fname = false;
bool	svg = false;
bool	size = true;
BString	filename;
BString	face;
BPath	path;
BString	gCharacters;

Emoconfig::Emoconfig(const char* xmlfile): BMessage()
{
	fEmoticonSize = 16.0; //default
	numfaces = 0;

	fParser = XML_ParserCreate(NULL);

	XML_SetUserData(fParser, this);
	XML_SetElementHandler(fParser, StartElement, EndElement);
	XML_SetCharacterDataHandler(fParser, Characters);

	//path!
	BPath p(xmlfile);
	p.GetParent(&path);

	// loading the config file..
	BFile* settings = new BFile(xmlfile, B_READ_ONLY);
	off_t size;
	settings->GetSize(&size);
	if (size) {
		void* buffer = malloc(size);
		size = settings->Read(buffer, size);
		XML_Parse(fParser, (const char*)buffer, size, true);
		free(buffer);
	}
	delete settings;

	if (fParser)
		XML_ParserFree(fParser);

	printf("Emoconfig: loaded %d faces\n", numfaces);

}

Emoconfig::~Emoconfig()
{

}

void
Emoconfig::StartElement(void * /*pUserData*/, const char* pName, const char** /*pAttr*/)
{
	//printf("StartElement %s\n",pName);
	BString name(pName);
	if (name.ICompare("emoticon") == 0) {
		faces = new BMessage();
		svg = false;
	} else if (name.ICompare("text") == 0 && faces) {
		valid = true;
	} else if (name.ICompare("file") == 0 && faces) {
		fname = true;
	} else if (name.ICompare("svg") == 0 && faces) {
		//		printf("File is SVG\n");
		svg = true;
	} else if (name.ICompare("size") == 0) {
		size = true;
		gCharacters = "";
	}
}

void
Emoconfig::EndElement(void* pUserData, const char* pName)
{
	//printf("EndElement %s\n",pName);
	BString name(pName);

	if (name.ICompare("emoticon") == 0 && faces) {
		//faces->PrintToStream(); //debug
		delete faces;
		faces = NULL;

	} else if (name.ICompare("text") == 0 && faces) {
		valid = false;
		faces->AddString("face", face);
		//printf("to ]%s[\n",face.String());
		face.SetTo("");

	} else if (name.ICompare("file") == 0 && faces) {
		//load file

		//compose the filename
		BPath p(path);
		p.Append(filename.String());
		BBitmap* icons = NULL;

		if ( !svg ) {
			//
			icons = BTranslationUtils::GetBitmap(p.Path());
		}

		//assign to faces;
		fname = false;

		//		printf("Filename %s [%s]\n",p.Path(),path.Path());
		if (!icons) return;

		int 		i = 0;
		BString s;
		while (faces->FindString("face", i, &s) == B_OK) {

			if (i == 0) {
				((Emoconfig*)pUserData)->menu.AddPointer(s.String(), (const void*)icons);
				((Emoconfig*)pUserData)->menu.AddString("face", s.String());
			}
			((BMessage*)pUserData)->AddPointer(s.String(), (const void*)icons);
			((BMessage*)pUserData)->AddString("face", s.String());
			((Emoconfig*)pUserData)->numfaces++;
			i++;

		}


	} else if (name.ICompare("size") == 0) {
		if ( size ) {
			((Emoconfig*)pUserData)->fEmoticonSize = atoi(gCharacters.String());
		}

		size = false;
	}

}

void
Emoconfig::Characters(void * /*pUserData*/, const char* pString, int pLen)
{
	BString f(pString, pLen);
	//printf("Characters %s\n",f.String());
	if (faces && valid) {
		f.RemoveAll(" ");
		f.RemoveAll("\"");
		if (f.Length() > 0)
			face.Append(f);
	} else if (fname) {
		f.RemoveAll(" ");
		filename = f;

	} else {
		gCharacters.Append(f);
	}
}



