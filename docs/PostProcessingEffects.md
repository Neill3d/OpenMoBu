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

The effects could be applied only in a camera view.

## Python tool to tweak post effects properties.
PostProcessingTool.py - you can find in PythonScripts/Startup
[![PostProcessingTool]()]()

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
