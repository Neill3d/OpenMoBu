# Surface Attachment Constraint

<figure><img src="../../.gitbook/assets/image.png" alt=""><figcaption></figcaption></figure>

A constraint to attach the transform of a given model to the deformable surface of another model.

{% hint style="info" %}
In order to bake the motion of a constrained model, you need to use the Hard Plot method (there is a corresponding script in the action scripts). This means we can't use a background bake solver; instead, we must evaluate the scene deformation of the source model on every frame. Hard Plot goes frame by frame, evaluates the scene, and writes keyframes for the constrained model.
{% endhint %}

## Video

I[ntroduction Video](https://youtu.be/M0MGZV39G1c?si=o9dfFC8QskG4cCRX)

## LOG

06.10.18 Updated and included into OpenMoBu

15.12.15 Fix attachment rotation issue by using TBN matrix

13.12.15 First workable version

26.11.15 Added barycentric coordinates to hold specified surface area, use normal then to compute new constrained rotation
