 Welcome to the Neill3d code repository!
====

 Here I'm sharing some C++ and Python code samples for Autodesk MotionBuilder.
 For more information please visit my blog - http://www.neill3d.com
 
---
 Binaries
 
 You can find some precompiled files here - https://drive.google.com/open?id=0B83XZ3TC_S6PcFZtSTJXZGk4NzQ&authuser=0
 
---
 How to compile
 
 Note! Projects and MoBu solution are created in VisualStudio 2010.
 
  1) Specify env variables with the path to your motionbuilder folder
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

6) I'm using one MotionCodeLibrary for compiling all the projects. In each project I have added a relative path to that folder. Right now this library is a colletion of header and source files, it's not a static or dynamic library. That means to compile your project you should not only include needed headers but also add corresponding source code files to your project.

 
---
  Contact
  
  Sergey Solohin (Neill3d)
  e-mail to: s@neill3d.com

  www.neill3d.com
