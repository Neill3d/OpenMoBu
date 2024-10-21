# How To Compile

### How To Compile and Build

Projects are configurated with CMake scripts to run from Src folder. The target MotionBuilder version is defined in the `src/PRODUCT_VERSION.txt`

From a given version in the file, MotionBuilder SDK default path is `C:/Program Files/Autodesk/MotionBuilder ${productversion}/OpenRealitySDK`

By default binaries will be copied into `MotionBuilder/bin/x64/plugins` folder

MotionBuilder 2025 is using Qt 6.5.3 and that will be needed to build some of the plugins.

#### Third party libraries

Third party directory contains dependency libraries

You also have to locate glew library include and lib directories

For PostProcess plugin uses freetype2 and freetype-gl in order to render a text. You have to locate these libraries. Pre-compiled libraries could be found in the project folder.

GPU Particles are using [nvFX library](https://github.com/tlorach/nvFX)&#x20;

### How to compile Old Notes

Note! Some old Projects and MoBu solution are created in VisualStudio 2010, the latest projects I've developed in VS 2015.

1. Specify env variables with the path to your motionbuilder folder

* ADSK\_MOBU\_{version number}\_64, for example ADSK\_MOBU\_2017\_64

NOTE: If during the compilation you have error "Macro definition of snprintf conflicts with Standard Library function declaration". The way out for the issue is to comment the line of declaration snprintf in kaydara.h

In order to render HUD text on top of post processed screen image, there is a setting "Draw HUD Layer" in the post process object. And the plugin have to be compiled with HUD\_FONT definition and with depencies of freetype2 and freetype-gl:

* freetype2 - https://github.com/aseprite/freetype2
* freetype-gl - https://github.com/frachop/freetypegl

2. After compiling release version of the plugin, there is a post build commandline to copy file into the output folder.

copy "$(ADSK\_MOBU\_2011\_64)\bin$(Platform)\plugins$(ProjectName).dll" "..\bin\_2011$(Platform)\plugins$(ProjectName).dll"

3. I'm using one MotionCodeLibrary for compiling all the projects. In each project I have added a relative path to that folder. Right now this library is a colletion of header and source files, it's not a static or dynamic library. You should not only include needed headers but also add corresponding source code files to your project.

### Compiling older plugins

There is a snprintf redifinition issue when compiling for older MoBu SDKs with VS2017+, to solve that you could just comment the line #define snprintf \_snprintf in kaydara.h in the OpenRealitySDK/include

1. On my machine I have put 32 bits version of MoBu into the 64 bits folders. They share the same files except folders bin/win32 and bin/x64.
2. For supporting older version of MoBu in different projects I'm using these pre-defines

* OLD\_FBEVALUATE\_LOCALTIME - MoBu 2010 doesn't support recieving system and local time from the evaluation variable FBEvaluateInfo
* OLD\_FBTIME\_SETFRAME - in old SetFrame function (MoBu 2010) you need to specify a bool parameter as an argument
* OLD\_FBTIME\_GETFRAME
* OLD\_SCENE\_EVENTS - There some new events in new version of MoBu like attach/detach component from FBScene
* OLD\_FBTREE - in old tree control you don't have several function to work with a checkboxes
* OLD\_FBAPP\_FILERENDER
* OLD\_FIND\_MODEL\_BY\_NAME - in old MoBu version we have different function name to get objects by name. Old version FBFindModelsByName, new version - FBFindModelByLabelName
* OLD\_DOCK\_POSITION
* OLD\_OPENGL\_HEADER - starting from MoBu 2011 we need to specify fbsdk-opengl header separately
* OLD\_FBMATH
* OLD\_CONSTRAINT\_MANAGER

3. You should know that each dll plugin project has a resource with a version information.
