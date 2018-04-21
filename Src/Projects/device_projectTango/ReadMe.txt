
 Project Tango Device plugin for Autodesk MotionBuilder

 Network spec

 one blocked TCP/IP socket to exchange with sync state (possible could be delta state)
 one non-blocked UDP socket to receive device pose packet
 one non-blocked UDP socket to send viewport image


 viewport image is OGL compressed and zip compressed
  OGL compression helps to quickly utilize it on a device GPU memory
  zip compression with mean rate 1:8, which is very important for global network bandwidth


Sergei <Neill3d> Solokhin 2018

GitHub page - https://github.com/Neill3d/OpenMoBu
Licensed under The "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE