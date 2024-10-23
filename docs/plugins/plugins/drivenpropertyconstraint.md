# Driven Property Constraint

The main use of this constraint is to transfer the value of a non-animatable property to an animatable property. For example, controlling the scale of a box based on the length of a curve path.

### How To Use:

* Add a Driven Property constraint to the scene.
* Add the object from which you want to read the property value into the Source group.
* Add the object to which you want to transfer the value into the Constrain group.
* Switch to the Setup tab in the Constraint UI Layout.
* Drag and drop the source object again, but this time into the first container.
* Drag and drop the constrain object into the second container.
* Select the property for both the source and the destination. An example is shown in the picture.
* Activate the constraint, and the value from the source object's property will be transferred to the destination object's property.

<figure><img src="../../.gitbook/assets/image (2) (1).png" alt=""><figcaption></figcaption></figure>

<figure><img src="../../.gitbook/assets/image (1) (1).png" alt=""><figcaption></figcaption></figure>

### Test Scenes

* [Camera Driven Property Example](../../../MB\_Scenes/constraint\_DrivenProperty\_Camera.fbx)
* [Curve Path Length Example](../../../MB\_Scenes/DrivenProperty\_CurvePathLength.fbx)

### Videos

* [Curve Path Length Example](https://drive.google.com/file/d/1QtuZqPx-RxS-W9v42Tz3nKA\_U8V3rR9x/view?usp=share\_link)
* [Simple Camera Property Example](https://www.youtube.com/watch?v=jSqzdz\_fkoA)

### Log

* 22.09.2013 – fix save/load issue in mobu 2014 version
* 07.05.2013 mobu 2014 64 bits
* 28.05.2012 mobu 2013 64 bit
* 11.11.2011 mobu 2012 64bit; fix displaying properties in UI when reset constraint; add load/save constraint properties.
