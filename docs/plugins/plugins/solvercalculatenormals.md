---
description: A solver to recalculate mesh normals on deformed meshes on fly
---

# Solver Auto Calculate Normals

MotionBuilder loads models with vertex normal information used for lighting. The quality issue arises when you deform such a mesh with a blendshape or skin binding. MotionBuilder only deforms the vertex positions, but not the normals. As a result, the deformations from blendshapes or skinning are not accurately reflected in the shape's lighting. The solver helps to recalculate normals on the fly, improving the lighting experience on deformed meshes.

<figure><img src="../../.gitbook/assets/image (3).png" alt=""><figcaption></figcaption></figure>

<details>

<summary>More examples</summary>

![](<../../.gitbook/assets/image (1) (1).png>)

![](<../../.gitbook/assets/normals\_solver\_01 (1).jpg>)

![](../../.gitbook/assets/normals\_solver\_02.jpg)

![](../../.gitbook/assets/normals\_solver\_03.jpg)

</details>

The solver automatically recalculates normals for models connected within the solver. It takes a tessellated mesh stored in GPU vertex buffers, then runs compute shaders to recalculate and replace the normals data.

All calculations are performed on the GPU and are not complex for modern graphics cards, so the solver should not impact your scene's performance significantly.

{% hint style="info" %}
Video card must have OpenGL 4.3 support in order to run the solver compute shaders
{% endhint %}

It does not take into account any smoothing group or hard edge threshold information, which is why it may not be suitable for some models. However, if your model has a uniform mesh with soft normals, it should work fine.

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
