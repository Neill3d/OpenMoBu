

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


#include "StringUtils.h"
#include <array>

FBString ExtractFileName(FBString str)
{
	FBString fname;
	int ndx = str.ReverseFind('\\');
	int ndx2 = str.ReverseFind('/');
	ndx = (ndx2 > ndx) ? ndx2 : ndx;

	if (ndx == -1) return str;
	fname = str.Mid( ndx+1, str.GetLen() - ndx );
	return fname;
}

FBString ExtractFilePath(FBString &str)
{
	FBString fpath = "";
	int ndx = str.ReverseFind('\\');
	if (ndx == -1) ndx = str.ReverseFind('/');
	if (ndx == -1) return fpath;
	else
	{
		fpath = str.Left( str.GetLen() - ndx );
	}
	return fpath;
}

bool SplitPath(FBString &path, FBStringList &list)
{
	if (path.IsEmpty() ) return false;

	std::array<char, 128> buffer;
	buffer.fill(0);
	std::array<char, 2> symbol;
	symbol.fill(0);
	unsigned int ndx = 0;
	unsigned int len = path.GetLen();
	while (ndx < len)
	{
		symbol[0] = path[ndx];
		if (symbol[0] == '\\' || symbol[0] == '/') 
			if (strlen(buffer.data()))
			{
				list.Add(buffer.data());
				buffer.fill(0);
				ndx++;
				continue;
			}
		
		strcat_s(buffer.data(), buffer.size(), symbol.data());
		ndx++;
	}

	if (strlen(buffer.data()) )
		list.Add(buffer.data());
	return true;
}

void ChangeFileName(FBString &str, FBString new_name)
{
	FBString fname;
	int ndx = str.ReverseFind('\\');
	if (ndx == -1) ndx = str.ReverseFind('/');
	if (ndx == -1) str = new_name;
	else
	{
		str = str.Left( str.GetLen() - ndx );
		str = str + new_name;
	}
}

void ChangeFileExt(FBString &str, FBString new_ext)
{
	if (str.GetLen() > 4 ) {
		str = str.Left( str.GetLen() - 4 );
	}
	str = str + new_ext;
}