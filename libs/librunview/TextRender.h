#ifndef _TextRender_H_
#define _TextRender_H_

#include <Font.h>

class TextRender
{
public:
	TextRender() {};
	virtual ~TextRender() {};

	virtual void     Render(BView* target, const char*, int num, BPoint pos) = 0;
	virtual void 	 GetHeight(font_height* height) = 0;
	virtual void		GetEscapements(const char charArray[], int32 numChars, float escapementArray[]) = 0;
	virtual float	 Size() = 0;
	//

};
#endif
