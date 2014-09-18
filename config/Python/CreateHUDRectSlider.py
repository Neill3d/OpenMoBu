#
# script for creating interactive HUD rect slider
# Sergey <Neill3d> Solohin
# e-mail to: s@neill3d.com
#   www.neill3d.com
#

# scripts works only with the first HUD element in the scene
#
#

from pyfbsdk import *

lScene = FBSystem().Scene

if lScene.HUDs.GetCount():
    HUD = lScene.HUDs[0]

    HUDRectElement = FBHUDRectElement("RectSlider")
    #Scene.ConnectSrc(HUD)          # Connect the HUD to the scene
    HUDTextElement.X = 0
    HUDTextElement.Y = 0
    HUDTextElement.Scale = 2
    HUDTextElement.Justification = FBHUDElementHAlignment.kFBHUDCenter
    HUDTextElement.HorizontalDock = FBHUDElementHAlignment.kFBHUDCenter
    HUDTextElement.VerticalDock = FBHUDElementVAlignment.kFBHUDTop
    HUDTextElement.PropertyAddReferenceProperty(HUD.PropertyList.Find("FreeMemory"))
    HUD.ConnectSrc(HUDTextElement) # Connect HUDTextElement to the HUD
    Scene.Cameras[0].ConnectSrc(HUD) # Connect to Perspective camera