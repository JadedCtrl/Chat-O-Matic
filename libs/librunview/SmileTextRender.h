#ifndef _SmileTextRender_H_
#define _SmileTextRender_H_

#include <Font.h>
#include <View.h>

#include <stdio.h>
#include <TranslationUtils.h>
#include <Resources.h>
#include <String.h>

#include <librunview/TextRender.h>
#include <librunview/Emoticor.h>


class SmileTextRender : public TextRender
{
public:

	SmileTextRender(): TextRender() {};

	virtual ~SmileTextRender() {};

	virtual void Render(BView* target, const char* txt, int num, BPoint pos)  {

		BBitmap* pointer = NULL;
		BString f(txt, num);

		if (Emoticor::Get()->Config()->FindPointer(f.String(), (void**)&pointer) == B_OK) {
			target->SetDrawingMode( B_OP_ALPHA );
			target->DrawBitmapAsync( pointer, BPoint(pos.x, pos.y - (Emoticor::Get()->Config()->GetEmoticonSize() / 2)) );
			target->SetDrawingMode( B_OP_OVER );
		}
	};


	virtual float Size() {
		printf("GETTING EMOTICOR SIZE!!!!\n");
		return Emoticor::Get()->Config()->GetEmoticonSize();
	}

	virtual void GetHeight(font_height* h) {
		h->descent = h->ascent = Emoticor::Get()->Config()->GetEmoticonSize() / 2;
		h->leading = 0;
	};

	virtual void
	GetEscapements(const char * /*charArray*/, int32 numChars, float escapementArray[]) {
		//font.GetEscapements(charArray,numChars,escapementArray);
		escapementArray[0] = 1;
		for (int i = 1; i < numChars; i++) escapementArray[i] = 0;
	}
};
#endif
