
from pyfbsdk import *

# change lightmap name for a selected texture
#   special case for using a lightmap with a super lighting shader

lSystem = FBSystem()
lScene = lSystem.Scene

numberOfTextures = 0

for lTexture in lScene.Textures:
    if lTexture.Selected:
        print lTexture.Name
        
        lOption, uvset = FBMessageBoxGetUserValue("Texture " + lTexture.Name, "UVSet Name:", "UVSet1", FBPopupInputType.kFBPopupString, "Ok", "Cancel")
        if 1 == lOption:
            print uvset
            
            prop = lTexture.PropertyList.Find('UVSet')
            if prop is not None:
                prop.Data = uvset
                
                numberOfTextures += 1
        else:
            break                
                
if numberOfTextures > 0:
    FBMessageBox("UVSet Name", "You have updated " + str(numberOfTextures) + " texture(s)", "Ok")
else:
    FBMessageBox("UVSet Name", "Please select a texture component before running a script", "Ok")

# Nnnn
    