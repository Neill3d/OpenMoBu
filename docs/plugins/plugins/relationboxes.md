# Relation Constraint Boxes

## Comet's PoseReader relation box

Rigging for correction joints

![PoseReaderBox](../../Plugins/relationBox\_PoseReader.jpg)

Category - OpenMobu

A relation constraint box is based on Comet's PoseReader plugin for Autodesk Maya. The idea is to calculate the activation factor when a given transform's rotation is within specified cone limits.

This can be very useful for rigging, especially when working with correction joints where activation is linked to a joint's specific orientation. For example, it can be used to correct the shoulder's movement when the arm is raised upward.

A test scene is provided to demonstrate a rig setup on the left shoulder of a skeleton.

[Example scene](../../../MB\_Scenes/relationBox\_PoseReader.fbx)

[Video on Youtube](https://youtu.be/kSSu6DMcXNI?si=dn8zvbA9cIwPQVcN)

## PoseTransform Box

You can create poses for an object that internally store the transformation matrix of that object. The relation box can then be used to calculate the final transform at runtime by blending between the stored transforms in the poses.

It supports up to 6 poses, which can be defined in the relation box properties (accessible in the property view of the selected relation box).

![PoseTransformBox](../../Plugins/box\_poseTransform.jpg)

Category - OpenMobu

[Sample scene - relationBox\_PoseTransform.fbx](../../../MB\_Scenes/relationBox\_PoseTransform.fbx)

## Look Rotation Box

Calculate a rotation to look at a given target point. This is similar to an Aim constraint, but in this case, it's presented in the form of a relation box.

![LookRotationBox](../../Plugins/relationBox\_LookRotation.jpg)

Category - OpenMobu

[Sample scene - relationBox\_LookRotation.fbx](../../../MB\_Scenes/relationBox\_LookRotation.fbx)

## RayIntersect Box

Calculate the intersection point between a ray and a mesh.

![RayIntersectBox](../../Plugins/relationBox\_RayIntersect.jpg)

Category - OpenMobu

[Sample scene - relationBox\_RayIntersect.fbx](../../../MB\_Scenes/relationBox\_RayIntersect.fbx)

## Quaternion math boxes

![QuaternionBoxes](../../Plugins/relationBox\_quaternionMath.jpg)

Boxes Category - Quaternion

List of boxes:

* Axis Rotation - make a quaternion of rotation of a given angle around given axis
* Euler To Quaternion - convert euler angles into quaternion rotation. The node has property Rotation Order for the given euler angles.
* Interpolate - interpolate between two quaternions
* Invert - invert the quaternion
* Look Rotation - It's similar to the functionality of an "aim constraint" where an object rotates to look at a target point, ensuring that its "up" orientation is correctly maintained.
* Mult - multiply two quaternions
* Normalize - make a unit quaternion
* Quaternion To Euler - convert from quaternion rotation into euler angles. The node has property Rotation Order for the given euler angles.
* RotateBy - rotate a given 3d vector by quaternion rotation

[Sample scene - relationBox\_QuaternionMath.fbx](../../../MB\_Scenes/relationBox\_QuaternionMath.fbx)

## RBF Box

**Radial Basis Function (RBF) is a method for interpolating values in multidimensional space.** In 3D graphics and skeleton rigging, RBFs are used to smoothly blend corrective deformations or poses based on joint rotations, ensuring natural and continuous skin deformation across a range of movements.

RBFs are particularly efficient for **multidimensional interpolation**, which is perfect for rigging systems that need to handle multiple influencing joints. For example, if the orientation of a joint depends on multiple axes (like bending and twisting), RBFs handle the blending of these orientations smoothly.

![RBFBox](../../Plugins/relationBox\_RBF.jpg)

Category - OpenMobu

Boxes:

* RBFVector - interpolate in 3 dimentional space
* RBFVector 4 - interpolate in 4 dimentional space

{% hint style="info" %}
In the `@openmobu.txt` configuration file, there is a property called **Box RBF** with a setting for **number of connectors**, where you can define how many input values you would like to interpolate. By default, it is set to 6 input values.
{% endhint %}

Box Properties:

* Function Type - Gaussian, Multiquadratic, Inverse Multiquadratic. defines the way of weight distribution.
* Height&#x20;
  * The **height** parameter controls the **amplitude** or **influence strength** of the RBF at the center of the radial function. It determines how strongly the RBF affects the interpolation at the point where the distance from the center is zero.
  * In simple terms, **height** defines the peak value of the function. A higher height means the RBF has a stronger influence in its immediate vicinity.
* Sigma&#x20;
  * The **sigma** parameter controls the **spread** or **width** of the RBF. It determines how quickly the influence of the RBF decays as you move away from the center.
  * A smaller sigma value makes the RBF more concentrated around its center, meaning the influence drops off sharply as you move away. A larger sigma value creates a more gradual influence over a wider area.
* Rotation Multiply

[Sample scene - relationBox\_RBF.fbx](../../../MB\_Scenes/relationBox\_RBF.fbx)

## Sphere Coords Box

Convert Euler Rotation into Sphere Coords. In that case we could avoid gimbal lock effect on rotation and do a relation based on a polar coords on a sphere surface. Could be a nice suit for rigging correction joints.

![SphereCoordsBox](../../Plugins/relationBox\_sphereCoords.jpg)

Category - OpenMobu

[Sample scene - relationBox\_SphereCoords.fbx](../../../MB\_Scenes/relationBox\_SphereCoords.fbx)

[Sample scene - relationBox\_SphereCoords\_ExtractRotationXZ.fbx](../../../MB\_Scenes/relationBox\_SphereCoords\_ExtractRotationXZ.fbx)

## Spring Box

spring dynamics, secondary motion

A box for relation constraint ![This is an image](../../Plugins/springbox.jpg)

Spring box math based on fundamental formule F = -kx

[A short demo video on Youtube](http://youtu.be/rAToQEmg\_LY)

[Sample scene - relationBox\_Spring.fbx](../../../MB\_Scenes/relationBox\_Spring.fbx)

[Sample scene - relationBox\_SpringAndCamera.fbx](../../../MB\_Scenes/relationBox\_SpringAndCamera.fbx)

| Parameter  | Description                                                                                                                                                                                                                                                                     | Default Value |
| ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------- |
| Stiff      | stiffness of the springs (the higher the value, the more rigid) Hardness is the reaction and the effect of the spring from which she draws a point in the original position.                                                                                                    | 0.1           |
| Damp       | the size of fading fluctuations spring (the more, the less hesitation commit spring) The essence of this parameter under a big question, in practice, it gives some the wrong result, so sometimes fading fluctuations can be regarded as a constant value of 0.10 for example. | 0.0           |
| Length     | the length of the spring (should be set to 0) A value of zero reference position is the point of going, otherwise the spring will be busy trying to position a specified distance from moving                                                                                   | 0.0           |
| Mass       | the mass. The more weight the more difficult it is to move the spring with a space or vice versa to return it to its original position.                                                                                                                                         | 1.0           |
| Friction   | force of resistance that allows “stifle” the dynamics of the spring and return it to its original position. The higher the number the less the effect of spring ”                                                                                                               | 0.1           |
| EvalSteps  | number of spring evaluation substeps per second                                                                                                                                                                                                                                 | 30.0          |
| RealTime   | checkbox to specify spring evaluation time. When checkbox is on, spring uses system time and play in realtime. When it’s off, spring uses player control local time from time slider (this is for plotting and animation playback).                                             | on (1.0)      |
| Zero Frame | frame on which we reset our simulation (for non realtime evaluation). This is when we rewind our animation to the very begining and want to start simulation from that “zero” point                                                                                             | 0 (frame 0)   |
| Pos        | input position                                                                                                                                                                                                                                                                  |               |
| Result     | output position                                                                                                                                                                                                                                                                 |               |
