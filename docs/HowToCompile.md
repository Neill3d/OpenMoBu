## How To Compile and Build

Projects are configurated with CMake scripts to run from Src folder.
The target MotionBuilder version is defined in the src/PRODUCT_VERSION.txt

From a given version in the file, MotionBuilder SDK default path is  C:/Program Files/Autodesk/MotionBuilder ${productversion}/OpenRealitySDK

By default binaries will be copied into MotionBuilder/bin/x64/plugins folder

Third party directory contains dependency libraries
You also have to locate glew library include and lib directories
For PostProcess plugin uses freetype2 and freetype-gl in order to render a text. You have to locate these libraries. Pre-compiled libraries could be found in the project folder.