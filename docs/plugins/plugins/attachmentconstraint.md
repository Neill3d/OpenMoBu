# Surface Attachment Constraint

 Constraint to attach a transform of a given model into a deformable surface of another model.

NOTE: in order to bake the motion of constrained model, you have to use a Hard Plot method (there is a correspondent script in action scripts), that means that we can't use a background bake solver, we have to evaluate scene deformation of a source model in every frame, so hard plot goes frame by frame, evaluates the scene and writes keyframes of a constraint model.

## Video

Introduction Video - https://youtu.be/M0MGZV39G1c?si=o9dfFC8QskG4cCRX
  
## LOG

 06.10.18
  Updated and included into OpenMoBu

 15.12.15
 Fix attachment rotation issue by using TBN matrix

 13.12.15
 First workable version

 26.11.15
 Added barycentric coordinates to hold specified surface area, use normal then to compute new constrained rotation