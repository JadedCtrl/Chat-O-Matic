#ifndef _NormalTextRender_H_
#define _NormalTextRender_H_

#include "TextRender.h"
#include <Font.h>
#include <View.h>

#include <stdio.h>

class NormalTextRender : public TextRender
{
    public:
    	  
    	 NormalTextRender(BFont f):TextRender(){
                        	font=f;
          }
          virtual ~NormalTextRender() {};
          
       virtual void     Render(BView *target,const char* txt,int16 num,BPoint pos)  {
           
           target->SetFont(&font);   
           target->DrawString(txt,num,pos); 
          
           
       }; 
       
       
       virtual float Size(){ return font.Size();}
       
       virtual void GetHeight(font_height *height){ font.GetHeight(height); };
    
    
	   virtual void		
	   GetEscapements(const char charArray[], int32 numChars,float escapementArray[])
	   {
  			font.GetEscapements(charArray,numChars,escapementArray);
	   }
    
    private:
            BFont       font;
            
};
#endif
