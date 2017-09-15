

	View 360 Tool

  In a custom 3d view visual control I'm processing 6 scene render passes from different cameras (pass per cubemap side).
 Then we compose 6 sides into a equirectangle panorama image

 Features:
  - render 6 sides and compose into a panorama
  - apply equirectangle shader for VR experience
  - toggle windowed / fullscreen mode
  - save current frame into a tif image

 TODO:
  - stereo modes don't work correctly
  - save timeline image sequence

 In code I'm using a framebuffer wrapper from Alex V. Boreskoff <alexboreskoff@mtu-net.ru>, <steps3d@narod.ru>

 equirectangleSphere shader - https://github.com/protyposis/Spectaculum


 == CONTACT INFORMATION ==

Author Sergey Solokhin (Neill3d) 2014-2017
 e-mail to: s@neill3d.com
   www.neill3d.com

 == LOG ==

 14.09.2017
  + start adding stereo side by side preview mode for VR experience

 26.08.2017
 + first public release