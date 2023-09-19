
=== Post Processing Manager ===

 Grabbing the viewer color and depth and applying some post effects on it.

 == Dependencies

 - Freetype_gl - fonts for emulated HUD system 
 https://github.com/rougier/freetype-gl

 - Freetype2

 - Shared Motion Library

 - post processing effects

//
//	Sergei <Neill3d> Solokhin 2018-2022
//
//	GitHub page - https://github.com/Neill3d/OpenMoBu
//	Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
//

LOG

17.09.2019
 + fixed depth attenuation for multi lights lens flare

09.04.2018
 + motion blur effect

23.03.2018
 + upper/lower clip
 + displacement effect

13.02.2018
 More testing, skip post processing for manipulators, emulate HUD elements to draw them on top

31.01.2018
 First Beta
 finished SSAO (added linearize pre-pass and bilateral blur, mix passes, fix some bugs)

30.01.2018
 Updated SSAO with HBAO algorithm

29.01.2018
 Create UI Tool, fixed many bugs and issues
 Added DOF, SSAO in progress

25.01.2018
 Added Lens Flare Effect
 Added Film Grain Effect

23.01.2018
 First build