# OpenMoBu code repository!

 It's a set of C++ and Python code for Autodesk MotionBuilder that I have developed while taking part in several projects.
 Another big set of plugins and scripts for Autodesk MotionBuilder you can find in MoPlugs repository.

Please post issues and feature requests to this [github repository issues section](https://github.com/Neill3d/OpenMoBu/issues)

[![MoPlugs](https://github.com/Neill3d/MoBu/blob/master/Documentation/Images/Bryan_usecase.jpg)]()

---
 Note! Please have a look at the repository Wiki pages for more information about setup and scripts/plugins

## Introduction webinars
* Part 1 - https://youtu.be/sgzunIbNcp4
* Part 2 - https://youtu.be/5By9h3KO6JE
* Part 3 - https://youtu.be/OcH3vusjros
* Part 4 - https://youtu.be/qNmFzHt0O0w

## Binaries
 
 There is an installation package in "Release" section of the repository.
 
 Also some old binaries you can find in "bin" folder.
 To setup custom plugins and scripts path, please, try a configuration application - https://github.com/Neill3d/MoBu_ConfigApp

## How to compile
 
 Note! Some old Projects and MoBu solution are created in VisualStudio 2010, the latest projects I've developed in VS 2013.
 
  1) Specify env variables with the path to your motionbuilder folder
* ADSK_MOBU_2010_64
* ADSK_MOBU_2011_64
* ADSK_MOBU_2012_64
* ADSK_MOBU_2013_64
* ADSK_MOBU_2014_64
* ADSK_MOBU_2015_64
* ADSK_MOBU_2016_64
* ADSK_MOBU_2017_64
 
 2) On my machine I have put 32 bits version of MoBu into the 64 bits folders. They share the same files except folders bin/win32 and bin/x64.

3) For supporting older version of MoBu in different projects I'm using these pre-defines

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

## Special Thanks

 Big thanks to Les Androids Associes! They gave me a permission to share my development to the public.

## Contact
  
  Sergei Solokhin (Neill3d)
  e-mail to: s@neill3d.com

  http://neill3d.com/en/
