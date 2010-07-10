#ifndef _Emoticor_h_
#define _Emoticor_h_


#include <String.h>
#include "RunView.h"
#include "Emoconfig.h"

class Emoticor
{
public:

	static Emoticor*	Get(); //singleton


	void		AddText(RunView* fTextView, const char* text,  int16 cols , int16 font , int16 cols2 , int16 font2 );
	void		LoadConfig(const char*);

	Emoconfig* Config();

	~Emoticor();

private:
	Emoticor();
	Emoconfig*	fConfig;
	void		_findTokens(RunView* fTextView, BString text, int tokenstart, int16 cols , int16 font , int16 cols2 , int16 font2);

};

#endif

