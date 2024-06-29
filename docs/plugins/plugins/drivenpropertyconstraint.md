# Driven Property Constraint

The main usage is to transfer value of some non animatable property into an animatable property. 
For example, to control a scale of a box by curve path length.

### How To Use

* Add a Driven Property constraint into the scene
* Put an object that you want to read property value from into a Source group
* Put an object that you want to transfer value into a Constrain group
* Switch to Setup tab in the Constraint UI Layout
* Drag and drop the source object once again but this time into a first container
* Drag and drop the constrain object into a second container
* Choose a property for the source and destination. An example is shown on the picture
* Activate the constraint and the value from the source object's property will be written into the destination object's property

![Example scene](https://github.com/Neill3d/OpenMoBu/blob/master/Documentation/Images/driven_property_constraint.jpg)

### Test Scenes

- [Camera Driven Property Example](https://github.com/Neill3d/OpenMoBu/blob/master/MB_Scenes/constraint_DrivenProperty_Camera.fbx)
- [Curve Path Length Example](https://github.com/Neill3d/OpenMoBu/blob/master/MB_Scenes/DrivenProperty_CurvePathLength.fbx)

### Videos

* [Curve Path Length Example](https://drive.google.com/file/d/1QtuZqPx-RxS-W9v42Tz3nKA_U8V3rR9x/view?usp=share_link)
* [Simple Camera Property Example](https://www.youtube.com/watch?v=jSqzdz_fkoA)

### Log

* 22.09.2013 – fix save/load issue in mobu 2014 version
* 07.05.2013
mobu 2014 64 bits
* 28.05.2012
mobu 2013 64 bit
* 11.11.2011
mobu 2012 64bit;
fix displaying properties in UI when reset constraint;
add load/save constraint properties.
