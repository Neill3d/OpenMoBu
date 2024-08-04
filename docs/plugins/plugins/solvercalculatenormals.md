---
description: A solver to recalculate mesh normals on deformed meshes on fly
---

# Solver Auto Calculate Normals

MotionBuilder loads model with vertex normal information used for lighting. The quality issue comes when you deform such mesh with a blendshape or skin binding. The MotionBuilder deforms only vertex position, but not normals. Because of that a deformation that comes from blendshapes or skinning, will not be reflected well on a shape lighting. The solver helps to recalculate normals on fly and improve the lighting experience on a deformed meshes.

<figure><img src="../../.gitbook/assets/image.png" alt=""><figcaption></figcaption></figure>

<details>

<summary>More examples</summary>

![](<../../.gitbook/assets/image (1).png>)

![](<../../.gitbook/assets/normals\_solver\_01 (1).jpg>)

![](../../.gitbook/assets/normals\_solver\_02.jpg)

![](../../.gitbook/assets/normals\_solver\_03.jpg)

</details>

The solver does an auto normals recalculation for connected models. It takes a model tesselated mesh which is stored on gpu vertex buffers, then run compute shaders to recalculate and replace normals data.

Calculations are fully on gpu, not complex for modern cards, so the solver should not impact your scene performance in general.

{% hint style="info" %}
Video card must have OpenGL 4.3 support in order to run the solver compute shaders
{% endhint %}

It doesn't take in a count any smoothing group or hard edges threshold information and that's why it could be not a good fit for some models. If your model has a nice uniform mesh with soft normals, that should work fine.

### How To Use

*   Locate a solver in the assets browser in solvers category

    ![assets](../../Plugins/solver\_normals\_assets.jpg)
* Add solver into the scene
* Assign models to the solver by using a model context menu or solver Affected Models property list. ![props](../../Plugins/solver\_normals\_props.jpg)

### Test Scenes

[FBX Scene with Deformable mesh and the solver](../../../MB\_Scenes/solver\_CalculateNormals.fbx)

### Videos

* [first release](https://youtu.be/QYUpd1u6O\_o?si=WzmfQbCdII8WR1oB)
* [first release WIP](https://youtu.be/YFLuMV-uGR8?si=pcvfAaf7Vm51x6vB)
