

####################################################
#
#   Additive Animation Tool.py
#
#   Script works with current character
#  Character should not have any input assigned (any Control Rig or Stance, etc.)
#   Please choose a take that has an animation you want to convert into additive
#  Then choose the base pose. It could be a first frame of the animation 
#   or it could be another animation in some other take that you should specify
#
#   When you press Compute Additive, the tool will create a new take
#    with base pose in zero animation layer and additive keyframes in layer 1
#   Button Extract Layer Into a New Take will copy keyframes from layer 1 into a new take
#    That will be a clear additive animation that you can use as a clip and then
#   import into a story or into a game engine.
#
#
#
# Author Sergey Solokhin (Neill) 2017
#   e-mail to: s@neill3d.com
# Homepage: www.neill3d.com
#
#   LOG
#   02.09.2017  converted into a tool
# 
##################################################

from pyfbsdk import *
from pyfbsdk_additions import *

import os.path
import sys

##################################### LIB CHECK CODE ###
gScriptFileName = "libAdditiveAnimation.py"

def SetupLibPath(libFilename):

    paths = FBSystem().GetPythonStartupPath()
    for path in paths:        
        if os.path.isfile(path + "\\" + libFilename):
            
            # check if that path is inside sys.path
            if not path in sys.path:
                sys.path.append(path)
    #
#
########################################################

# lib should be one of python startup folders
SetupLibPath(gScriptFileName)

import libAdditiveAnimation

if (sys.version_info > (3, 0)):
    import importlib
    importlib.reload(libAdditiveAnimation)
else:
    reload(libAdditiveAnimation)

gAdditiveAnimation = libAdditiveAnimation.CAdditiveAnimation()

###################################################################
## TOOL

def OnButtonBaseTypeClick(control, event):
    gAdditiveAnimation.SetBaseFromAnimationFirstFrame( control.State == 1 )

def EventAnimTakeChange(control, event):
    gAdditiveAnimation.SetAnimationTakeIndex( control.ItemIndex )
    
def EventBaseTakeChange(control, event):
    gAdditiveAnimation.SetBaseTakeIndex( control.ItemIndex )

def OnButtonComputeClick(control, event):
    
    if False == gAdditiveAnimation.ComputeCharacterAdditive():
        FBMessageBox( "Additive Tool", "Please Set Any Character As Current", "Ok" )
        
def OnButtonExtractClick(control, event):
    
    if False == gAdditiveAnimation.ExtractAdditiveLayerIntoANewTake():
        FBMessageBox( "Additive Tool", "Please Make Sure Current Character Is Assigned\n"
            "  And Current Take contains additive animation layer\n", "Ok" ) 

def OnButtonAboutClick(control, event):
    FBMessageBox( "Additive Tool", "Author Sergey Solokhin (Neill3d) 2017\n"
        "GitHub: Neill3d\n e-mail to: s@neill3d.com\n www.neill3d.com", "Ok" )

def PopulateLayout(mainLyt):
    
    lSystem = FBSystem()
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)

    b = FBLabel()
    b.Caption = 'Choose an animation take'
    main.Add(b, 24)
    
    b = FBList()
    b.Style = FBListStyle.kFBDropDownList
    for take in lSystem.Scene.Takes:
        b.Items.append(take.Name)
    b.OnChange.Add( EventAnimTakeChange )
    main.Add(b, 24)

    b = FBLabel()
    b.Caption = ''
    main.Add(b, 24)
    
    b = FBButton()
    b.Style = FBButtonStyle.kFBCheckbox
    b.Caption = 'Use Animation First Frame As Base Pose'
    b.State = 1
    b.OnClick.Add( OnButtonBaseTypeClick )
    main.Add(b, 24)
    
    b = FBLabel()
    b.Caption = 'Or Choose a base pose take'
    main.Add(b, 24)
    
    b = FBList()
    b.Style = FBListStyle.kFBDropDownList
    for take in lSystem.Scene.Takes:
        b.Items.append(take.Name)
    if len(b.Items) > 1:
        b.ItemIndex = 1
    b.OnChange.Add( EventBaseTakeChange )
    main.Add(b, 24)
    
    b = FBLabel()
    b.Caption = ''
    main.Add(b, 24)
    
    b = FBButton()
    b.Caption = 'Compute Additive'
    b.OnClick.Add( OnButtonComputeClick )
    main.Add(b, 24)
    
    b = FBButton()
    b.Caption = "Extract Take Layer as a New Take"
    b.OnClick.Add( OnButtonExtractClick )
    main.Add(b, 24)
    
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add( OnButtonAboutClick )
    main.Add(b, 24)
    
gDEVELOPMENT = False
    
def CreateTool():    
    print("create tool")
    global t
    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Additive Animation Tool v." + str(gAdditiveAnimation.GetVersion()))
    PopulateLayout(t)
    t.StartSizeX = 250
    t.StartSizeY = 320
    
    if gDEVELOPMENT:
        ShowTool(t)

CreateTool()