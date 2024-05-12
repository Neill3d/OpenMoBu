# References Manager

 The solution for a reference manager contains of 2 plugins (reference manager and fbx extension), one command line application ShadingGraph, python reference tool with extended functionality and set of python action scripts to perform varios operations over the referenced model.

 The functionality of legacy reference system is very limited, for example it's not supported shaders applied on a models. The solution provided by the plugins and scripts could help to deal with that
 - you can reference fbx model with assigned shaders (from a list of supported standard shaders)

 The fbx extension plugin fixes the bug when you are loading a scene with a referenced model that could not be located by a stored path. In that case mobu just locks the file, while the extension helps to unlock and onload the referenced model and you can define a new path once the scene is loaded.

 The solution doesn't give any functionality to support control rig, it's still a limitation of a reference system.

Extend context menu for the references object in the scene
- save edits
- restore edits
- bake edits - bake shader graph edits to a referenced model
- change path - change a referenced model file path
- reload - reload a referenced model from the source file
- delete - delete a referenced model from a scene

## Python scripts

 Main utility - MBFileRefAdvanced.py, tool and its dialogs are based on Qt

Additional scripts which are used by the reference manager to execute different actions on a given (selected) referenced model in the scene

- ActionBakeEdits.py
- ActionChangePath.py
- ActionDelete.py
- ActionLoad.py
- ActionReferenceAFile.py
- ActionReLoad.py
- ActionRestoreEdits.py
- ActionSaveEdits.py
- ActionTemp.py - template script to experiment

 Additional scripts used by the main utility
 - FbxShadersGraphBake.py
 - FbxShadersGraphImport.py
 - FbxShadersGraphMisc.py


## ShadingGraph console application
 From a given fbx file the utility extracts scene shaders information into xml formatted text.

 You can run application in extract or bake modes

In an extract mode the utility is going to load an fbx scene, look for "Shader" object classes and write information about them into xml file

In a bake mode the utility is going to load a special FbxShadersClasses.fbx file where template shaders are stored and then look for an fbx scene from the input file provided and merge shaders according to the given xml shaders input file. The result fbx scene will be stored into a given output file path.

 command line arguments
 -e or -extract or -f to run in extract mode
 -b or -bake to run in bake mode

 Optional list of arguments that you can define by providing file path is
 - fbx file path
 - xml file path
 - output path

