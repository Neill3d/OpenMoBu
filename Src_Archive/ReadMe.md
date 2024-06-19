
## PLUGINS

* box_poseReader - this is a conversion of original Comet's PoseReader plugin for Maya
* box_rayIntersect - relation constraint box that helps to project ray on a mesh and get a contact point and normal
* box_spring - relation constraint box to simulate simple spring
* constraint_driven - animate properties which are non-animatable by default
* constraint_NewtonPhysics - old physics constraint based on Newton physics engine
* filter_extrapolate - pre and post animation curve extrapolation (based on hermite splines)
* impexp_csm - character studio marker format support
* impexp_csv - optitrack text format support
* manager_AutoSave - plugin for 2009 MoBu. 
There are three features: 
	1) Auto save animation of current character to a specified folder
	2) Adding a submenu to the MoBu main menu using win api hook
	3) Key shortcuts for the FCurves window (changing keyframe tangents)
* manip_HUDext - extend funtionality of a HUD view elements
* manip_skelet - move bone pivot with affecting the heirarchy
* misc_VolumeCalculator - calculate your optical active zone in the virtual room
* tool_graphView_sample - simple own implementation of a graph view visual component based on FBView class
* tool_Hand - controls for easier fingers selection and some parametric functionality for hand animation
* tool_renderAdvance - tool to render up to 4 views in a frame
* tool_SplitStyle - example of making a splitter control between two UI regions
* tool_UIBuilder - tool for faster making a UI for c++ and python
* tool_view360 - real-time render into a equirectangular panorama
* tool_viewBarelDistortion - stereo camera 2 panes side by side render with barel distortion post-effect
* tool_ViewTwoPanes - an example to render 2 different cameras into 2 panes. That could be a start point for doing your VR experiment
* tool_windowsmenu_sample - sample for MoBU under 2011. using winApi for making a context menu
* userobject_normalsSolver - solver for real-time computing soft normals for models. Could be useful to improve visual quality for skeleton binded and blendshape animated meshes

### How to compile

1) Specify env variables with the path to your motionbuilder folder

Depends on your motionbuilder version, use one of the names:
* ADSK_MOBU_2010_64
* ADSK_MOBU_2011_64
* ADSK_MOBU_2012_64
* ADSK_MOBU_2013_64
* ADSK_MOBU_2014_64
* ADSK_MOBU_2015_64

2) On my machine I have put 32 bits version of MoBu into the 64 bits folders. They share the same files except folders bin/win32 and bin/x64.

3) For supporting older version of MoBu in different projects I'm using these predefines

* OLD_FBEVALUATE_LOCALTIME - MoBu 2010 doesn't support recieving system and local time from the evaluation variable FBEvaluateInfo
* OLD_FBTIME_SETFRAME - in old SetFrame function (MoBu 2010) you need to specify a bool parameter as an argument
* OLD_FBTIME_GETFRAME
* OLD_SCENE_EVENTS - There some new events in new version of MoBu like attach/detach component from FBScene
* OLD_FBTREE - in old tree control you don't have several function to work with a checkboxes
* OLD_FBAPP_FILERENDER
* OLD_FIND_MODEL_BY_NAME - in old MoBu version we have different function name to get objects by name. Old version FBFindModelsByName, new version - FBFindModelByLabelName
* OLD_DOCK_POSITION
* OLD_OPENGL_HEADER - starting from MoBu 2011 we need to specify fbsdk-opengl header separately
* OLD_FBMATH
* OLD_CONSTRAINT_MANAGER

4) You should know that each dll plugin project has a resource with version information.

5) After compiling release version of the plugin, there is a command to copy file into the installation folder.

copy "$(ADSK_MOBU_2011_64)\bin\$(Platform)\plugins\$(ProjectName).dll" "..\bin_2011\$(Platform)\plugins\$(ProjectName).dll"

Note! In some plugins I'm using post-build commands to copy GLSL shaders from project folder into a motionbuilder plugins folder

6) I'm using the MotionCodeLibrary for compiling all the projects. In each project I have added a relative path to that folder. Right now this library is a colletion of header and source files, it's not a static or dynamic library. That means to compile your project you should not only include needed headers but also add corresponding source code files to your project.

### Contact

  Sergey Solokhin (Neill3d) 2014-2017
  
	e-mail to: s@neill3d.com
	
		www.neill3d.com
