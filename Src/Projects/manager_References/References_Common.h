
#pragma once

// References_Common.h
/*
Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
*/

// TODO: look inside the PYTHONPATH env var for paths and search for scripts

#define ACTION_DEV_PATH			"D:\\References"

#define ACTION_REFAFILE_MENU	"&Reference A File..."
#define ACTION_REFAFILE			"\\MBFileRefAdvanced\\ActionReferenceAFile.py"

#define ACTION_LOAD				"\\MBFileRefAdvanced\\ActionLoad.py"
#define ACTION_LOAD_TEMP		"\\ActionLoadTemp.py"

#define ACTION_UNLOAD			"\\MBFileRefAdvanced\\ActionUnLoad.py"
#define ACTION_UNLOAD_TEMP		"\\ActionUnLoadTemp.py"

#define ACTION_RENAME			"\\MBFileRefAdvanced\\ActionReName.py"
#define ACTION_RENAME_TEMP		"\\ActionReNameTemp.py"

#define ACTION_DELETE_MENU		"Delete A File Reference"
#define ACTION_DELETE			"\\MBFileRefAdvanced\\ActionDelete.py"
#define ACTION_DELETE_TEMP		"\\ActionDeleteTemp.py"

#define ACTION_RELOAD_MENU		"Reload A File Reference"
#define ACTION_RELOAD			"\\MBFileRefAdvanced\\ActionReLoad.py"
#define ACTION_RELOAD_TEMP		"\\ActionReLoadTemp.py"

#define ACTION_SAVEEDITS_MENU	"Save Edits As..."
#define ACTION_SAVEEDITS		"\\MBFileRefAdvanced\\ActionSaveEdits.py"
#define ACTION_SAVEEDITS_TEMP	"\\ActionSaveEditsTemp.py"

#define ACTION_RESTORE_MENU		"Restore Shaders Graph To Load State"
#define ACTION_RESTORE			"\\MBFileRefAdvanced\\ActionRestoreEdits.py"
#define ACTION_RESTORE_TEMP		"\\ActionRestoreEditsTemp.py"

#define ACTION_BAKE_MENU		"Bake Shaders Graph Edits"
#define ACTION_BAKE				"\\MBFileRefAdvanced\\ActionBakeEdits.py"
#define ACTION_BAKE_TEMP		"\\ActionBakeEditsTemp.py"

#define ACTION_CHANGEPATH_MENU	"Change Reference File Path..."
#define ACTION_CHANGEPATH		"\\MBFileRefAdvanced\\ActionChangePath.py"
#define ACTION_CHANGEPATH_TEMP	"\\ActionChangePathTemp.py"

/////////////////////////////////////////////////////////////

const char *GetActionPath();
const char *GetSystemTempPath();

bool FindAScriptPath();

bool FileExists(const char *szPath);