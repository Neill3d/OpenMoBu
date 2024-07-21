# Camera Links Group Visibility

This is a manager to control a group visiblity by activation of a camera. When a camera has an additional property "LinkedGroup", then a group with such name will toggle its visibility based on a camera being current in the viewport or not.

![cameraLinkVisManager](../../Plugins/cameraLinkVis\_manager.jpg)

## Python Tool Camera Linking Tool

A tool that helps to track a relations between cameras and groups and automatically assign data into "LinkedGroup" user property in a camera.

![cameraLinkVisTool](../../Plugins/cameraLinkVis\_tool.jpg)

When you press Refresh, it shows a current list of cameras and groups in the scene.

Then you have to select a camera that you would like to link with a group and select a group. When the link is established, then every next time you select the same camera, the correspondent assigned group should be selected as well.

The tool you can find in PythonScripts\Startup\CameraAndGroupLinkingTool.py

### Samples

test scene - MB\_Scenes\manager\_CameraLinksGroupVisibility.fbx
