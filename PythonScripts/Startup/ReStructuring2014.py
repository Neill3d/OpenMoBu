
#
# Script for keeping objects transform animation while changing heirarchy
#
#  Version for MotionBuilder 2014
#
# Sergey Solohin (Neill3d) 2016
#   e-mail to: s@neill3d.com
#  www.neill3d.com
#

from pyfbsdk import *
from pyfbsdk_additions import *
import os.path
import sys

##################################### LIB CHECK CODE ###
gScriptFileName = "libReStructuring.py"

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
import libReStructuring

if (sys.version_info > (3, 0)):
    import importlib
    importlib.reload(libReStructuring)
else:
    reload(libReStructuring)

gRestructuring = libReStructuring.CRestructuring()

gLabelStatus = FBLabel()

gFileName = 'D:/Restructuring_temp.fbx'

#######################################################

def EventButtonHold(control, event):
    gLabelStatus.Caption = 'Prepare for Hold operation'
    mList = FBModelList()   
    FBGetSelectedModels(mList)
    
    gRestructuring.Hold(mList)
    
    gLabelStatus.Caption = 'Hold Finished'
    
def EventButtonFetch(control, event):
    gLabelStatus.Caption = 'Prepare for Fetch operation'
    gRestructuring.Fetch()
    gLabelStatus.Caption = 'Fetch Finished'
    
def EventButtonSave(control, event):
    gLabelStatus.Caption = 'Prepare for saving to file'
    gRestructuring.SaveToFile(gFileName)
    gLabelStatus.Caption = 'SaveToFile Finished'
    
def EventButtonLoad(control, event):
    gLabelStatus.Caption = 'Prepare for loading from file'
    gRestructuring.LoadFromFile(gFileName)
    gLabelStatus.Caption = 'LoadFromFile Finished'
    
def EventButtonAbout(control, event):
    FBMessageBox( 'Restructuring Tool', 'Author Sergey Solokhin (Niell3d) \n May 2017', 'Ok' )

def PopulateLayout(mainLyt):
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Hold animation for selection"
    b.OnClick.Add(EventButtonHold)
    main.AddRegion("hold", "hold", x, y, w, h)
    main.SetControl("hold", b)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"hold")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Save To File"
    b.OnClick.Add(EventButtonSave)
    main.AddRegion("savetofile", "savetofile", x, y, w, h)
    main.SetControl("savetofile", b)

    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"savetofile")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Load From File"
    b.OnClick.Add(EventButtonLoad)
    main.AddRegion("loadfromfile", "loadfromfile", x, y, w, h)
    main.SetControl("loadfromfile", b)

    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"loadfromfile")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "Fetch animation"
    b.OnClick.Add(EventButtonFetch)
    main.AddRegion("fetch", "fetch", x, y, w, h)
    main.SetControl("fetch", b)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"fetch")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add(EventButtonAbout)
    main.AddRegion("about", "about", x, y, w, h)
    main.SetControl("about", b)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"about")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")

    gLabelStatus.Caption = "Status"
    main.AddRegion("status", "status", x, y, w, h)
    main.SetControl("status", gLabelStatus)
    
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Restructuring Tool")
    PopulateLayout(t)
    t.StartSizeX = 400
    t.StartSizeY = 320
    ShowTool(t)


CreateTool()





