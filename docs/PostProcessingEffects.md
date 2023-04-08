# Post Processing Effects
 It's a part of OpenMoBu project and could be found in OpenMoBu repository on Github
 
The plugin that applies post effects in your viewport window
[![MoPlugs](https://github.com/Neill3d/MoBu/blob/master/Documentation/Images/Bryan_usecase.jpg)]()
List of available effects:
* color correction
* displacement
* Depth of Field (DOF)
* Film Grain
* Fish Eye
* Lens Flare
* Motion Blur
* Screen Space Ambient Occlusion (SSAO)
* Vignetting

## How To Start
The effects could be applied only in a camera view.

In order to apply effects, you have to create a post process user object which contains all properties. You can do that from Assert Browser or from the Post Process Tool

The use of python tool will be shown in the next topic, let's do that from assets browser.
[![PostProcessingTool](https://github.com/Neill3d/OpenMoBu/blob/master/docs/post_process_asset.jpg)]()

Drag'n'drop into your scene, once created you can always locate from Navigate window in User Objects section
[![PostProcessingTool](https://github.com/Neill3d/OpenMoBu/blob/master/docs/post_process_user_object.jpg)]()

Switch the view to a camera view (or create a new camera if you don't have any camera in the scene).
Select the post process user object and in property view you will get access to its properties to tweak.
[![PostProcessingTool](https://github.com/Neill3d/OpenMoBu/blob/master/docs/post_process_user_object_properties.jpg)]()

Here you can active the desired effects and the whole effect system in order to see the result in real-time in the viewport.

## Python tool to tweak post effects properties.
PostProcessingTool.py - you can find in PythonScripts/Startup
[![PostProcessingTool](https://github.com/Neill3d/OpenMoBu/blob/master/docs/post_process_tool.jpg)]()

## Post Effects User Object
 This is a container of post effects properties and that's the object in the scene that you can use for merging and tweaking manually by u

## Post Effects and HUD
 The post effects modify the final image, where HUD elements are draw as part of it. In order to recover original HUD, the post process plugin has special feature to emulate the HUD drawing on top of post processed image. That is not a full 100% replication of MotionBuilder HUD system, but it covers most of needs with printing out text, rectangles.

## Additional features of the plugin
* stream view image via UDP socket (used for virtual camera communication), was designed for a Google Tango tablet - https://www.youtube.com/watch?v=WpLeU2rrf_Y
* trigger evaluation of Python Script devices during rendering. That feature is always enabled and it helps to trigger Embedded Python Scripts while running an offline rendering.

## Videos about the plugin:
* [Introduction Video from a live stream](https://www.youtube.com/watch?v=sgzunIbNcp4&t=4887s)
* [Update with new features from 2020](https://youtu.be/AOKk90_bzX4)
