
/////////////////////////////////////////////////////////////////////////////////////////
//
// Licensed under the "New" BSD License. 
//		License page - https://github.com/Neill3d/MoBu/blob/master/LICENSE
//
// GitHub repository - https://github.com/Neill3d/MoBu
//
// Author Sergey Solohin (Neill3d) 2014
//  e-mail to: s@neill3d.com
//		www.neill3d.com
/////////////////////////////////////////////////////////////////////////////////////////

#include "TextUtil.h"
#include <string>

int gTextPos;
int gTextM;

int GetTextParts(char *s)
{
	gTextPos = 0;
	gTextM = 0;

	if (!s)
		return 0;

	int res=0;
	int len=strlen(s);
	bool text=false;
	
	for (int i=0; i<len; i++) {
		if (s[i]==' ' || s[i]=='\n' ) {
			if (text) { res++; text=false; }
		} else text = true;
	}

	if (text) res++;

	return res;
}



// n from 1 to ...
int GetTextPartLen(char *s, int n)
{
	int l = 0;
	int res=0;
	int len=strlen(s);
	bool text=false;
	
	for (int i=0; i<len; i++) {
		if (s[i]==',' || s[i]=='\n') {
			if (text) { res++; text=false; }
		} else {
			text = true;
			
			if (res == (n-1)) l++;
		}
	}

	return l;
}
/*
void GetTextPart(char *s, int n, char *part)
{
	int m=0;
	int res=0;


	int	count=0;

	for (int i=0; i<strlen(s); i++)
		if (s[i]!=',') {
			if (m == n) part[count++] = s[i];
			res=1;
		} else if (res == 1) {
				m++;
				res=0;
			}

	part[count] = '\0';
}
*/



void GetTextPart( const char *s, int n, char *part )
{
	int i=gTextPos;
	int res=0;
	int	count=0;

	for ( ; i<strlen(s); i++)
	{
		if (gTextM > n) break;

		if (s[i]!=' ') {
			if (gTextM == n) part[count++] = s[i];
			res=1;
		} else if (res == 1) {
				gTextM++;
				res=0;
			}
	}
	gTextPos = i;
	part[count] = '\0';
}

char *CutTextPart(char *s, int n)
{
	int l = GetTextPartLen(s, n);
	int res=0;
	int len = strlen(s);
	bool text=false;
	char *part = new char[len-l];

	l=0;

	for (int i=0; i<len; i++) {
		if (s[i]==',' ) {
			if (text) { text=false; res++; part[l++]=s[i];}
		} else {
			text = true;
			
			if (res != (n-1)) 
				part[l++]=s[i];
		}
	}

	return part;
}

char *TranslateString (char *buf)
{
	static	char	buf2[32768];
	int		i, l;
	char	*out;

	l = strlen(buf);
	out = buf2;
	for (i=0 ; i<l ; i++)
	{
		if (buf[i] == '\n')
		{
			*out++ = '\r';
			*out++ = '\n';
		}
		else
			*out++ = buf[i];
	}
	*out++ = 0;

	return buf2;
}