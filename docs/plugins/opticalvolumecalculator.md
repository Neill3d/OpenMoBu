# Optical Volume Calculator

Plugin for setup active zone for optical cameras

A user object to calculate an optical capture volume score by providing a capture space, cameras with defined field of view and far distance. A score is calculated where at least two cameras see a point inside the capture space

![opticalVolumeCalculator](../Plugins/VolumeCalculator\_volume.jpg)

## Work with the plugin

Set the cameras with the real optical parameters (FOV) in the scene. Set the object VolumeCalculator and adjust its parameters – size of the room for capturing and cameras for calculating.

You can dispose the cameras as you want and whereever you want inside of a virtual room and calculate active zone with the help of a button Solve in VolumeCalculator properties. The task is to get the best filling of the active zone and to get the highest mark in the parameter “Mark”.

## Model Properties

![opticalVolumeProperties](../Plugins/VolumeCalculator\_properties.jpg)

* Length – lenght of the room (1 unit – 1cm)
* Width – width of the room (1 unit – 1cm)
* Height – height of the room (1 unit – 1cm)
* Show room – switch room visibility
* Show points – switch volume points visibility
* Step – step to calculate the point of space
* Min Coverage – the minimum number of cameras is to cover a point of space to be in the active zone (it’s good to be no less than 3)
* Marker – test object for checking the projection the rays from cameras inside of the room.
* Cameras – cameras, which are used in processing
* Solve – for calculation of the inner volume
* Marks – rating of the inner volume
* Show zone – display active zone on the floot of the room.
* Circle zone – for switching over between squeare of round zone on the floor of the room.
* Zone X Offset – indent of the active zone display on the floor off the center.
* Zone Z Offset – indent of the active zone display on the floor off the center.
* Zone Length – display of the active zone size – length
* Zone Width – display of the active zone size – width

## Parameters for Optitrack V100(R2) cameras

Horizontal FOV – 46,2

Far distance plane – 700 (7 meters)

Resolution 640×480

## Usage guide

* Parameters Length, Width, Height should be set according to real numbers of the room. Cameras have difference in their horizontal orientation and vertical orientation, so do not forget to turn and twist them.
* Display of the active zone on the floor of the room and its parameters are conventional and do not do with the active zone calculation by the plug-in. This display should be used as a supplement to understand the result of the calculation and marking of the active zone further.
* Do not do the calculation step too short. A big number of points requires a lot of resource and may lead to crash of the programme.
* The marker can de used for the rating of the active zone, to move it and to watch the projection rays from the cameras.
* To feel comfortable you can set double window in MotionBuilder viewer. In one part you can switch on the display with the camera, in the other one – a perspective view on the room. The display of the active zone should be switched on only for a current camera, which you’re working with. There is additional button “Switch to Camera” to make this scenario easier. The button switches over the first view on the selected camera and switches on the display of the active zone for only this camera. ![volumeCalculateInWork](../Plugins/VolumeCalculator\_inwork.jpg)

Examples of capture volume setups could be found in the MB\_Scenes/OpticalVolumeCalculator folder

[Video On Youtube](https://youtu.be/DlW6bF2QhM8?si=fjI9PwNVj5SfvSsY)

## Log

28.01.2013 Add resolution and lens presets for Optitrack cameras.
