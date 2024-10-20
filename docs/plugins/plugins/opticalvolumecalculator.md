# Optical Volume Calculator

Plugin for setup active zone for optical cameras

A user object that calculates the optical capture volume score by providing a capture space and cameras with defined fields of view and far distances. The score is calculated based on the points within the capture space that are visible to at least two cameras.

![opticalVolumeCalculator](../../Plugins/VolumeCalculator\_volume.jpg)

## Work with the plugin

Place the cameras with real optical parameters (FOV) in the scene. Add the VolumeCalculator object and adjust its parameters, such as the size of the capture room and the cameras to be used for calculation. You can position the cameras freely within the virtual room and calculate the active zone using the 'Solve' button in the VolumeCalculator properties. The goal is to maximize the active zone coverage and achieve the highest score in the 'Mark' parameter.

## Model Properties

![opticalVolumeProperties](../../Plugins/VolumeCalculator\_properties.jpg)

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

* The parameters for Length, Width, and Height should be set to match the real dimensions of the room. Cameras differ in their horizontal and vertical orientations, so be sure to rotate and adjust them as needed.
* The display of the active zone on the room's floor and its parameters are for reference only and are not part of the actual active zone calculation by the plug-in. This display should be used as an aid to understand the calculation results and active zone marking.
* Avoid making the calculation step too small. A large number of points can consume a lot of resources and may cause the program to crash.
* The marker can be used to rate the active zone, move it, and observe the projection rays from the cameras.
* For ease of use, you can set up a double window in the MotionBuilder viewer. In one view, you can display the camera feed, and in the other, a perspective view of the room. The active zone display should be enabled only for the current camera you're working with. To simplify this process, there is an additional 'Switch to Camera' button. This button switches the first view to the selected camera and activates the display of the active zone for that camera only. ![volumeCalculateInWork](../../Plugins/VolumeCalculator\_inwork.jpg)

Examples of capture volume setups could be found in the MB\_Scenes/OpticalVolumeCalculator folder

[Video On Youtube](https://youtu.be/DlW6bF2QhM8?si=fjI9PwNVj5SfvSsY)

## Log

28.01.2013 Add resolution and lens presets for Optitrack cameras.
