
#pragma once

//--- SDK include
#include <fbsdk/fbsdk.h>

// find a pointer to a viewer and panes components
void PrepViewerComponents();
void GetViewerPaneInfo(int &paneCount, FBCamera *&pCameraPane0, FBCamera *&pCameraPane1);