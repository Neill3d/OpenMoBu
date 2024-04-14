
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: neill3d@gmail.com
// 
/////////////////////////////////////////////////////////////////////////////////////////

#ifndef TextUtil_h_
#define TextUtil_h_

#include <windows.h>
//#include <stdlib.h>
//#include <stdio.h>


int GetTextParts(char *s);
// n from 1 to ...
int GetTextPartLen(char *s, int n);
//void GetTextPart(char *s, int n, char *part);
void GetTextPart(const char *s, int n, char *part);
char *CutTextPart(char *s, int n);

char *TranslateString (char *buf);

#endif
