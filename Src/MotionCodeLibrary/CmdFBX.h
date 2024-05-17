
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: CmdFBX.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "FBXUtils.h"

//#ifdef CMD_SEND_CODE
bool CmdMakeSnapshotFBX_Send(const char *filename, const char *uniqueName, InputModelData &data, const bool ResetXForm);
//#endif

bool CmdMakeSnapshotFBX_Receive(InputModelData *data);