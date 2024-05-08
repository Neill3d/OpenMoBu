## How To Install

### Where to find compiled plugins
You can find installation package or individual compiled binaries in the [release section of the repository](https://github.com/Neill3d/OpenMoBu/releases)

Scripts are splitted into 
- Startup which are tools or code libraries
- Action scripts which are single execution script files

[In Scenes folder on the github repository you can find example cases of plugins use](https://github.com/Neill3d/OpenMoBu/tree/master/MB_Scenes)

For motionbuilder the default path for plugins is `<motionbuilder>/bin/x64/plugins` 

### Custom plugins and script path
Environment variables to define custom search path
- `MOTIONBUILDER_PLUGIN_PATH` for plugins 
- `MOTIONBUILDER_PYTHON_STARTUP` for scripts
  
[There is a repository with application MoBu_ConfigApp](https://github.com/Neill3d/MoBu_ConfigApp) that could help to define these paths with GUI.
