
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

#pragma once

#include "windowsubmenu.h"

enum
{
#define	MENUCLASS(menuName, menuType, menuText)	MENU_##menuName,
	#include "MainMenuItems.h"
#undef	EDITCLASS
};

class	MyMenuHandle	: public MenuHandle
{
public:
	MyMenuHandle()
	{
		MenuItem	item;

#define MENUCLASS(menuName, menuType, menuText) item.id = (int)MENU_##menuName; \
	item.type = menuType;	sprintf(item.text, ""##menuText); \
	mItems.Add(item);
#include "MainMenuItems.h"
#undef EDITCLASS

	}

	int		ItemPress(int item)
	{
		switch(item)
		{
		case	MENU_NewBox:
			{
				// fill current trackable marker with data from the FBOpticalMarker
				HFBModel	pBoxModel = (HFBModel) new FBModelCube("NewBox");
				pBoxModel->Show = true;
			}break;
		case MENU_NewPlane:
			{
				HFBModel	pBoxModel = (HFBModel) new FBModelPlane("NewPlane");
				pBoxModel->Show = true;
			}break;
		case MENU_SelBox:
			{
				FBModelList	pList;
				FBFindModelsOfType( pList, FBModelCube::TypeInfo );
				for (int i=0; i<pList.GetCount(); i++)
				{
					HFBModel	pModel  = pList[i];
					if (pModel)
					{
						pModel->Selected = true;
					}
				}
			}break;
		case MENU_SelPlane:
			{
				FBModelList	pList;
				FBFindModelsOfType( pList, FBModelPlane::TypeInfo );
				for (int i=0; i<pList.GetCount(); i++)
				{
					HFBModel	pModel  = pList[i];
					if (pModel)
					{
						pModel->Selected = true;
					}
				}
			}break;
		case MENU_ClearSel:
			{
				FBModelList	pList;
				FBGetSelectedModels(pList);

				for (int i=0; i<pList.GetCount(); i++)
				{
					HFBModel	pModel  = pList[i];
					if (pModel)
					{
						pModel->FBDelete();
						pModel = NULL;
					}
				}
			}break;
		}

		return 1;
	}
	MenuItem		*ItemCollect(int *count)
	{
		*count	= mItems.GetCount();
		return mItems.GetArray();
	}

private:
	FBArrayTemplate	<MenuItem>	mItems;
};