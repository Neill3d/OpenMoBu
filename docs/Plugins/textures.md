# Textures Plugins

## How to use

 Create a new texture object from the assets browser. You can do that by drag n drop an asset on the object or empty space in the scene.
 Some texture objects like advance blend require to have two textures (layers) in order to operate. You have to drag n drop two textures on them and connect as layers

 Every texture object is running own render target. In case one of the properties is animated or some input relation is connected, the texture will be evaluated at run-time. For a texture object like parameteric blend that is needed to play sprite texture animations.

## Texture Advance Blend

 The texture object for blending two images with advanced number of composite modes and optional masking for the second image

### Properties

 - Background color - clear color, animatable parameter, should be keyed in order to evaluate a texture object at run-time
 - CustomComposition - a flag that activate the advance composite logic
 - Blend Mode Advanced - a wide range of composition blend modes
 - Use Mask - a flag if the mask should be used
 - Mask - a mask texture

## Texture Parameric Blend

 A texture object to player sprite texture animations.
 This is a plugin that introduced a new texture object to unpack sprite altal into a run-time texture
If you want to see the animated effect, you have to put key on BackgroundLayer property

### Properties

- BackgroundLayer - clear color, animatable parameter, should be keyed in order to evaluate a texture object at run-time
- CustomComposition - a toggle to enable the whole parametric blend logic
- Sprite Order - type of packing vector or matrix. Blended matrix is a way to interpolate between nearest frames
- Count U - number of packed textures in a row in the atlas
- Count V - number of packed texture in  a colomn in the atlas
- Param U - current parameter to define position in a row to display
- Param V - current parameter to define position in a column to display
- Premult Alpha - whould we multiply output texture alpha with average rgb color
- Sprite Animation - a toggle to enable auto play of sprite animation
- Sprite FPS - number of frames per second to play the sprite
- Sprite Frames Limit - should 100, the total progress upper limit
- Sprite Local Play - a toggle between real-time or play-time animation play
- Sprite Local Start Time - start time for a play-time animation play
- Sprite Loop Play - a toggle to switch between single or looped play

## LOG

30.11.2015
 + added mask for advance blend texture

26.11.2015
 + automatic sprite animation in a vector sprite sheet
 + blended matrix shader

25.11.2015
 + use composite master shaders for blending two textures with advance list of modes
 + sprite order styles - vector, matrix, blended matrix

24.11.2015
 + param blend
 + using GL_NV_blend_equation_advanced for supporting blend mode list