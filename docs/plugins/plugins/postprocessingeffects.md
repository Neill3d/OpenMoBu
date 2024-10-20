# Post Processing Effects

It's a part of OpenMoBu project and could be found in OpenMoBu repository on Github

The plugin that applies post effects in your viewport window [![MoPlugs](https://github.com/Neill3d/MoBu/blob/master/Documentation/Images/Bryan\_usecase.jpg)](postprocessingeffects.md)&#x20;

List of available effects:

* Color correction
* Displacement
* Depth of Field (DOF)
* Film Grain
* Fish Eye
* Lens Flare
* Motion Blur
* Screen Space Ambient Occlusion (SSAO)
* Vignetting

## How To Start

The effects could be applied only in a camera view.

In order to apply effects, you have to create a post process user object which contains all properties. You can do that from Assets Browser or from the Post Process Tool. In Assets Browser the user object is located in `Assets / Templates / Shading Elements`

The use of python tool will be shown in the next topic, let's do that from assets browser. ![PostProcessingTool](../../../post\_process\_asset.jpg)

Drag'n'drop into your scene, once created you can always locate from Navigate window in User Objects section ![PostProcessingTool](../../../post\_process\_user\_object.jpg)

Switch the view to a camera view (or create a new camera if you don't have any camera in the scene). Select the post process user object and in property view you will get access to its properties to tweak. ![PostProcessingTool](../../../post\_process\_user\_object\_properties.jpg)

Here you can active the desired effects and the whole effect system in order to see the result in real-time in the viewport.

## Supported drag'n'drop operations

* if you drag camera / light / null, they will connected into Data Camera / Lens Flare Light / DOF Focus Object
* if you drag one post process user object into another, the values from on object will be copied into another one
* if Camera is assigned in a correspondent property of a post process object, then post processing effect become a local, just for a specified camera
* Post procesing effect applies only on user camera and skip any system camera and Perspective as well
* Better to not combine Camera Effects (Antialiasing, Depth Of Field) with post processing effects `For Antialiasing better to use render option. BUT the option conflicts with Reflection shader, so you will not see reflections!`

## Default Configuration

Default values for the properties are stored in `My Documents \ MB \ 20xx-x64 \ config`

Name of a configuration file is `< PC Name >.PostProcessing.txt`

## Python tool to tweak post effects properties.

PostProcessingTool.py - you can find in PythonScripts/Startup ![PostProcessingTool](../../../post\_process\_tool.jpg)

## Effects Common Notes

* DOF and SSAO and any other effect which is using depth buffer could be sensible to camera near/far planes
* GLSL Shader stores value in single float, so please take care about too big values for the far plane

## DOF

In DOF Effect you can manually specify a focal distance.

If Null is assigned in the Focus Object Property, Focal distance will be computed automatically.

When Use Camera DOF Properties is on, then Focal Distance and Focal Range will be taken from Camera Real-Time effects

To increase a quality of DOF, try to insrease values for Samples and Rings

## Lens Flare

PosX and PosY are percent of screen size (width and height)

## Post Effects and HUD

The post effects modify the final image, where HUD elements are draw as part of it. In order to recover original HUD, the post process plugin has special feature to emulate the HUD drawing on top of post processed image. That is not a full 100% replication of MotionBuilder HUD system, but it covers most of needs with printing out text, rectangles.

## Additional features of the plugin

* stream view image via UDP socket (used for virtual camera communication), was designed for a Google Tango tablet - [Introduction To Virtual Camera](https://www.youtube.com/watch?v=WpLeU2rrf\_Y)
* trigger evaluation of Python Script devices during rendering. That feature is always enabled and it helps to trigger Embedded Python Scripts while running an offline rendering.

## Videos about the plugin:

* [Introduction Video from a live stream](https://www.youtube.com/watch?v=sgzunIbNcp4\&t=4887s)
* [Update with new features from 2020](https://youtu.be/AOKk90\_bzX4)

## Developer Guide

The plugin third party dependencies

* glew
* freetype
* freetype-gl
* bzip2
* libpng
* zlib

All library binaries for windows x64 are included in the project folder.

## LOG

### 17.09.2019

* fixed depth attenuation for multi lights lens flare

### 09.04.2018

* motion blur effect

### 23.03.2018

* upper/lower clip
* displacement effect

### 13.02.2018

More testing, skip post processing for manipulators, emulate HUD elements to draw them on top

### 31.01.2018

First Beta finished SSAO (added linearize pre-pass and bilateral blur, mix passes, fix some bugs)

### 30.01.2018

Updated SSAO with HBAO algorithm

### 29.01.2018

Create UI Tool, fixed many bugs and issues Added DOF, SSAO in progress

### 25.01.2018

Added Lens Flare Effect Added Film Grain Effect

### 23.01.2018

First build
