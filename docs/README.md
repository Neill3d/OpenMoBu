# OpenMoBu documentation

Collection of scripts and plugins for Autodesk MotionBuilder

## Introduction Videos

[Stream Video Part 1](https://www.youtube.com/live/sgzunIbNcp4?si=kTXzdfnZ3FnkiIdv)

## Pages

 Articles on different topics

[How To Install](HowToInstall.md)

[How To Compile/Build](HowToCompile.md)

[Script Device](ScriptDevice.md)

## Relation Constraint Boxes

 Those plugins are extended functionality of relation constraint which is a form of visual programming with node system and connections between nodes. The plugins add new nodes to the constraint.

- [Comet's PoseReader Box](Plugins/PoseReader.md)
- [PoseTransform Box](Plugins/PoseTransformBox.md)
- [RayIntersect Box](Plugins/RayIntersectBox.md)
- [RBF Box](Plugins/RBFBox.md)
- [Spring Box](Plugins/SpringBox.md)

## Constraint Plugins

A set of plugins that constains source object / objects with a destination object / objects with a defined logic
 For example, parent/child, aim, surface attachment, etc.
 Constraint computed result has a priority over the animation keyframe data.

- [Attachment Constraint](Plugins/attachmentConstraint.md)
- [Driven Property Constraint](Plugins/drivenPropertyConstraint.md)
- [Twist Extraction Constraint](Plugins/twistExtractionConstraint.md)

## Devices

 A plugin which handle some external Input/Output with real-time data flow and nodes constraints

### FaceCap Device

A device plugin for live streaming via OSC from FaceCap iPhone App
For more information, please visit [FaceCap official website](https://bannaflak.com/face-cap/index.html)

## View Manipulators

 A plugin which hook the user input operations with the viewport in order to perform some action in the scene like transforming objects by dragging manipulator handle with the mouse input

[Lock Camera Manipulator](lockCameraManipulator.md)

## Misc

 Tools, custom objects in the scene

### Optical Volume Calculator

 The custom model that helps to calculate a unit score of a volume coverage for the optical system. In order to run the solver, you have to immitate the capturing space by placing cameras the same way how you are going to place them in the real room with a correct defined FOV and aspect ratio.

[Read More...](opticalVolumeCalculator.md)

## Graphics

 Shading, fx, geometry preparation for rendering

### Post Processing Filters

 A special user object, that helps to add post process filters on top of your camera view.

[Read More...](Plugins/PostProcessingEffects.md)

### Super Dynamic Lighting Shader

 This is an advance version of Dynamic Lighting Shader with features like vertex displacement, specular map, Matcap shading, etc.

[Read More...](Plugins/superDynamicLightingShader.md)

### Calculate Normals Solver

 A solver to recalculate mesh normals on fly during rendering. That helps to better visualize deformations from blendshape activation or by skinned skeleton transform.

[Read More...](Plugins/solverCalculateNormals.md)

## About the project

Sergei (Neill3d) Solokhin

Special thanks to Les Androids Associes.

[Support the project](https://ko-fi.com/neill3d)
