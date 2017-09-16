
#include "utils.h"

FBComponent *gMainViewer=nullptr;
FBComponent *gPane0 = nullptr;
FBComponent *gPane1 = nullptr;

void PrepViewerComponents()
{
	gMainViewer = gPane0 = gPane1 = nullptr;

	FBScene *pScene = FBSystem::TheOne().Scene;

	for (int i=0, count=pScene->Components.GetCount(); i<count; ++i)
	{
		const char *compName = pScene->Components[i]->Name;
		if ( 0 == strcmp(compName, "MainViewer" ) )
			gMainViewer = pScene->Components[i];
		else if ( 0 == strcmp(compName, "ModelView0") )
			gPane0 = pScene->Components[i];
		else if ( 0 == strcmp(compName, "ModelView1") )
			gPane1 = pScene->Components[i];

		if (nullptr!=gMainViewer && nullptr!=gPane0 && nullptr!=gPane1)
			break;
	}

}


///////////////////////////////////////////////////////////
//

void GetViewerPaneInfo(int &paneCount, FBCamera *&pCameraPane0, FBCamera *&pCameraPane1)
{
	paneCount = 1;

	pCameraPane0 = FBSystem::TheOne().Renderer->CurrentCamera;
	pCameraPane1 = nullptr;

	if (nullptr != gMainViewer && nullptr != gPane0 && nullptr != gPane1)
	{
		// pane count
		FBProperty *pProp = gMainViewer->PropertyList.Find("PaneCount");
		paneCount = pProp->AsInt() + 1;

		// pane 1 cam
		pProp = gPane1->PropertyList.Find( "CurrentCamera" );
		if (nullptr != pProp)
		{
			for (int i=0, count=pProp->GetSrcCount(); i<count; ++i)
			{
				FBPlug *pPlug = pProp->GetSrc(i);

				if ( FBIS(pPlug, FBCamera) )
				{
					pCameraPane1 = (FBCamera*) pPlug;
					break;
				}
			}
		}
	}
}

