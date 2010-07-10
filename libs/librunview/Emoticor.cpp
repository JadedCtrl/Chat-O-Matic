#include "Emoticor.h"
#include <Resources.h>
#include <Directory.h>
#include <Path.h>
#include "string.h"

static Emoticor*	fInstance = NULL;


Emoticor*
Emoticor::Get()
{
	if (fInstance == NULL)
		fInstance = new Emoticor();

	return fInstance;
}

Emoticor::Emoticor()
{
	fInstance = NULL;
	fConfig = NULL;
}

Emoticor::~Emoticor()
{
	if (fConfig)
		delete fConfig;
}

Emoconfig*
Emoticor::Config()
{
	return fConfig;
}

void
Emoticor::LoadConfig(const char* txt)
{
	fConfig = new Emoconfig(txt);
}

void
Emoticor::_findTokens(RunView* fTextView, BString text, int tokenstart, int16 cols , int16 font , int16 cols2 , int16 font2)
{
	//******************************************
	// "Iteration is human, recursion is divine"
	//******************************************


	int32	newindex = 0;
	BString	cur;
	int	i = tokenstart;

	if (fConfig != NULL) {
		while (fConfig->FindString("face", i, &cur) == B_OK)
			//for(int i=tokenstart;i<config->numfaces;i++)
		{
			i++;
			//if(config->FindString("face",i,&cur)!=B_OK) return;

			newindex = 0;

			while (true) {
				newindex = text.IFindFirst(cur.String(), 0);
				//printf("Try %d %s  -- match %d\n",i,cur->original.String(),newindex);

				if (newindex != B_ERROR) {
					//take a walk on the left side ;)

					//printf("Found at %ld \n",newindex);

					if (newindex - 1 >= 0) {
						BString left;
						text.CopyInto(left, 0, newindex);
						//printf("ready to recourse! [%s]\n",left.String());
						_findTokens(fTextView, left, tokenstart + 1, cols, font, cols2, font2);
					}


					text.Remove(0, newindex + cur.Length());

					//printf("remaning [%s] printed  [%s]\n",text.String(),cur->original.String());

					fTextView->Append(cur.String(), cols2, cols2, font2);

					if (text.Length() == 0) return; //useless stack
				} else
					break;

			}
		}
	}

	fTextView->Append(text.String(), cols, cols, font);

}

void
Emoticor::AddText(RunView* fTextView, const char* txt,  int16 cols , int16 font , int16 cols2 , int16 font2 )
{

	BString left(txt);

	//	if(!fConfig)
	//		fTextView->Append(txt,cols,cols,font);

	_findTokens(fTextView, left, 0, cols, font, cols2, font2);

	return;

}

