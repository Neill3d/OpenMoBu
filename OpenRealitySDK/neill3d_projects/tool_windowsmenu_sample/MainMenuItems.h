
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

// test menu
#ifdef	MENUCLASS
MENUCLASS(NewBox, MF_STRING, "New box")
MENUCLASS(NewPlane, MF_STRING, "New plane")
MENUCLASS(Sep1, MF_SEPARATOR, "")
MENUCLASS(SelBox, MF_STRING, "Select all boxes")
MENUCLASS(SelPlane, MF_STRING, "Select all planes")
MENUCLASS(Sep2, MF_SEPARATOR, "")
MENUCLASS(ClearSel, MF_STRING, "Delete selected")
#endif