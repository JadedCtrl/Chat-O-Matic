#ifndef _Emoticor_h_
#define _Emoticor_h_


#include <String.h>
#include "RunView.h"
#include "Emoconfig.h"

class Emoticor
{
public:

	static Emoticor*	Get(); //singleton


	void		AddText(RunView* fTextView, const char* text,  rgb_color cols,
						rgb_color font, rgb_color cols2, rgb_color font2);
	void		LoadConfig(const char*);

	Emoconfig* Config();

	~Emoticor();

private:
	Emoticor();
	Emoconfig*	fConfig;
	void		_findTokens(RunView* fTextView, BString text, int tokenstart,
							rgb_color cols, rgb_color font, rgb_color cols2,
							rgb_color font2);

};

#endif

