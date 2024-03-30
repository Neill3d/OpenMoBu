[![Downloads](https://img.shields.io/github/downloads/neill3d/openmobu/2022/total.svg)](https://github.com/neill3d/openmobu/releases/tag/2022)
[![Downloads](https://img.shields.io/github/downloads/neill3d/openmobu/2019.1/total.svg)](https://github.com/neill3d/openmobu/releases/tag/2019.1)
<a href="https://discord.gg/EU5E2p9VCf"><img alt="OpenMoBu on Discord" src="https://img.shields.io/badge/discord-OpenMoBu-blue?logo=discord&logoColor=white&style=flat"></a>
# OpenMoBu code repository!
https://discord.gg/EU5E2p9VCf
 It's a set of C++ and Python code for Autodesk MotionBuilder that I have developed while taking part in several projects.
 Another big set of plugins and scripts for Autodesk MotionBuilder you can find in MoPlugs repository.

Please post issues and feature requests to this [github repository issues section](https://github.com/Neill3d/OpenMoBu/issues)

[![MoPlugs](https://github.com/Neill3d/MoBu/blob/master/Documentation/Images/Bryan_usecase.jpg)]()

---
 Note! Please have a look at the repository Wiki pages for more information about setup and scripts/plugins

[Documentation pages](http://neill3d.github.io/OpenMoBu/)

## Introduction webinars
* Part 1 - https://youtu.be/sgzunIbNcp4
* Part 2 - https://youtu.be/5By9h3KO6JE
* Part 3 - https://youtu.be/OcH3vusjros
* Part 4 - https://youtu.be/qNmFzHt0O0w

## Installation Package

 There are some ready to use installation packages, you can find them in a ["Release" section of the repository](https://github.com/Neill3d/OpenMoBu/releases).

## Binaries
 
 If you want to install manually or copy just updated binary, you can find precompiled binaries in the "bin" folder.
 
 To setup custom plugins and scripts path, please, try a configuration application - https://github.com/Neill3d/MoBu_ConfigApp

## How to compile
 
 Note! Some old Projects and MoBu solution are created in VisualStudio 2010, the latest projects I've developed in VS 2015.

  1) Specify env variables with the path to your motionbuilder folder
* ADSK_MOBU_{version number}_64, for example ADSK_MOBU_2017_64
 
 NOTE: If during the compilation you have error "Macro definition of snprintf conflicts with Standard Library function declaration". The way out for the issue is to comment the line of declaration snprintf in kaydara.h
 
 In order to render HUD text on top of post processed screen image, there is a setting "Draw HUD Layer" in the post process object. And the plugin have to be compiled with HUD_FONT definition and with depencies of freetype2 and freetype-gl:
 * freetype2 - https://github.com/aseprite/freetype2
 * freetype-gl - https://github.com/frachop/freetypegl
 
2) After compiling release version of the plugin, there is a post build commandline to copy file into the output folder.

copy "$(ADSK_MOBU_2011_64)\bin\$(Platform)\plugins\$(ProjectName).dll" "..\bin_2011\$(Platform)\plugins\$(ProjectName).dll"

3) I'm using one MotionCodeLibrary for compiling all the projects. In each project I have added a relative path to that folder. Right now this library is a colletion of header and source files, it's not a static or dynamic library. You should not only include needed headers but also add corresponding source code files to your project.

## Compiling older plugins

There is a snprintf redifinition issue when compiling for older MoBu SDKs with VS2017+, to solve that you could just comment the line #define snprintf _snprintf in kaydara.h in the OpenRealitySDK/include

1) On my machine I have put 32 bits version of MoBu into the 64 bits folders. They share the same files except folders bin/win32 and bin/x64.

2) For supporting older version of MoBu in different projects I'm using these pre-defines

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
 
3) You should know that each dll plugin project has a resource with a version information.

## Supporters and Special Thanks

A gold supporter for the repository and special thanks to Les Androids Associes!

## Contact
  
  Sergei Solokhin (Neill3d)
  e-mail to: neill3d@gmail.com

  https://www.facebook.com/Neill3D
