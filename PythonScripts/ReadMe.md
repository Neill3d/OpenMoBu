## Autodesk MotionBuilder script samples by Neill3d


### Actions

#### Animation

* CharTimeToZero.py - move animation to the start of timeline (zero)
* ComputeAccel.py - for selected models script computes positions acceleration and write into "accel" property
* CutLeft.py - to cut TRS keys from a start of a timeline upto current slider time (for all selected models)
* CutRight.py - to cut TRS keys from a current slider time upto end of a timeline (for all selected models)
* FinalizeOnSkeleton.py - plot on skeleton and remove a control rig
* MoveCharToZero.py - move character to the world zero
* ReCreateRig.py - plot on skeleton, remove control rig, create a new control rig and plot animation on it


#### HUDext

* CreateHUDRectSlider.py - create interactive HUD rect slider
* HUDextUI.py - tool to setup interative HUD elements for the HUDext plugin
* HUDextUI.ui - a file for UIBuilder tool

#### ImportExport

* BVH_Tool.py - BVH animation import/export tool 
* bvh.py - BVHReader class

#### Misc

* CharacterFace.py - just a small test, nothing important

#### SceneManagment

* ConnectRelationNodes.py -
* handlestart.py - put a null with a name "handle" and run the script to make Handle pivot point for selected modesl
* handlefinish.py - remove a Handle pivot point (created with handlestart)

* ComputeJointLength.py - for each selected model create a 'JointLength' property and write a distance to the model parent
* MakeCircle.py - script to create a circle 3d Path Model

### Lib

* libAdditiveAnimation.py - logic to compute additive layer by subtracting one take from another
* libReStructuring.py - logic to manage massive scene reparenting and keeping animation trajectories
* libUnitTests.py - manage batch of scene tests and scenarios. make a screenshot, record a video and write out a performance counters

### Tools

* AdditiveAnimationTool.py - UI for libAdditiveAnimation
* ReStructuring2014.py - UI for libReStructuring
* StayOnFloor_2013.py - tool to hold TRS channels movement to avoid foot sliding
* renamer.py - extended renamer tool for selected models

