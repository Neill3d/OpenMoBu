
//*****************************************************************************
//*****************************************************************************
// Name: TextUtil
// Autor: Solokhin Sergey
// Date: 18/12/2003
// Disc:	Процедуры и функции для работы с текстом,
//		разбиением его на части
//*****************************************************************************
//*****************************************************************************

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
