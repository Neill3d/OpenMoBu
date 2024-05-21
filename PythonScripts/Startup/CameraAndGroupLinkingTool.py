
#########################################################
##
## Camera and Group Linking Tool script for Autodesk MotionBuilder
##
## This script is an addon to the camera_linkvis plugin (moplugs project)
##  
## Github repo - https://github.com/Neill3d/MoPlugs
## Licensed under BSD 3-clause
##  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
##
##
## Author Sergey Solohin (Neill3d) 2014
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
#########################################################

from pyfbsdk import *
from pyfbsdk_additions import *

gDevelopment = True

lApp = FBApplication()
lSystem = FBSystem()

ListCameras = FBList()
ListGroups = FBList()

###########################################################

def RefreshGroups():
    ListGroups.Items.removeAll()
    ListGroups.Items.append( "..." )

    for lGroup in lSystem.Scene.Groups:
        ListGroups.Items.append( lGroup.Name )

    ListGroups.ItemIndex = 0

def OnRefresh(control, event):
    ListCameras.Items.removeAll()
    ListCameras.Items.append( "..." )

    for lCamera in lSystem.Scene.Cameras:
        if lCamera.SystemCamera == False:
            ListCameras.Items.append( lCamera.Name )
     
    ListCameras.ItemIndex = 0
     
    RefreshGroups()   
    
def FindCameraGroupName(lCamera):
    
    lProp = lCamera.PropertyList.Find("LinkedGroup")
    if lProp:
        return lProp.Data
        
    return ""
    
def AddCameraGroup(lCamera, lGroup):
    
    lProp = lCamera.PropertyList.Find("LinkedGroup")
    if lProp:
        lProp.Data = lGroup.Name
    else:
        lProp = lCamera.PropertyCreate( "LinkedGroup", FBPropertyType.kFBPT_charptr, "String", False, True, None )
        if lProp:
            lProp.Data = lGroup.Name
    
def ClearCameraGroup(lCamera):
    lProp = lCamera.PropertyList.Find("LinkedGroup")
    if lProp:
        lProp.Data = ""
    
def OnCameraSelected(control, event):
    print ("camera selected")
    
    # try to find selected camera and find appropriate group for it
    
    RefreshGroups()
    
    cameraName = ""
    if ListCameras.ItemIndex >= 0:
        cameraName = ListCameras.Items[ListCameras.ItemIndex]
    
    for lCamera in lSystem.Scene.Cameras:
        if lCamera.Name == cameraName:
            groupName = FindCameraGroupName(lCamera)    
            if groupName != "":
                print(groupName)
                for i in range(len(ListGroups.Items)):
                    if ListGroups.Items[i] == groupName:
                        ListGroups.ItemIndex = i
                        break
            #
    #
    
def OnGroupSelected(control, event):
    print ("group selected")

    groupName = ""
    if ListGroups.ItemIndex >= 0:
        groupName = ListGroups.Items[ListGroups.ItemIndex]
    
    currCamera = 0
    currGroup = 0
    
    cameraName = ""
    if ListCameras.ItemIndex >= 0:
        cameraName = ListCameras.Items[ListCameras.ItemIndex]
    
    for lCamera in lSystem.Scene.Cameras:
        if lCamera.Name == cameraName:
            currCamera = lCamera
            break
    
    for lGroup in lSystem.Scene.Groups:
        if lGroup.Name == groupName:
            currGroup = lGroup
            break
            
    if currCamera != 0:
        if currGroup != 0:
            AddCameraGroup(currCamera, currGroup)
        else:
            ClearCameraGroup(currCamera)
    
def OnNextLink(control, event):
    FBMessageBox( "Camera Linking Tool", "This function is in progress", "Ok" )
    
def OnPrevLink(control, event):
    FBMessageBox( "Camera Linking Tool", "This function is in progress", "Ok" )
    
def OnClear(control, event):
    FBMessageBox( "Camera Linking Tool", "This function is in progress", "Ok" )
    
def OnAbout(control, event):
    FBMessageBox( "Camera Linking Tool", "MoPlugs Project (C) 2014", "Ok" )

def PopulateTool(mainLyt):

    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)

    # create horizontal boxes packed in a vbox
    hstripes = FBVBoxLayout()
    
    # Shows how to create a FBHBoxLayout that grows from left to right
    box = FBHBoxLayout(FBAttachType.kFBAttachLeft)
    names = ["Refresh", "Next Link", "Prev Link", "Clear All", "About"]
    events = [ OnRefresh, OnNextLink, OnPrevLink, OnClear, OnAbout ]
    
    i = 0
    for name in names:
        b = FBButton()
        b.Caption = name
        b.OnClick.Add(events[i])
        
        box.AddRelative(b, 0.2)
        
        i += 1
        
    hstripes.Add(box, 35)

    # add list titles
    box = FBHBoxLayout(FBAttachType.kFBAttachLeft)
    names = ["Cameras", "Groups"]
    for name in names:
        b = FBLabel()
        b.Caption = name
        box.AddRelative(b, 0.5)
    hstripes.Add(box, 35)

    # add lists
    box = FBHBoxLayout(FBAttachType.kFBAttachLeft)

    ListCameras.Style = FBListStyle.kFBVerticalList
    ListCameras.OnChange.Add( OnCameraSelected )
    box.AddRelative(ListCameras, 0.5)

    ListGroups.Style = FBListStyle.kFBVerticalList
    ListGroups.OnChange.Add( OnGroupSelected )
    box.AddRelative(ListGroups, 0.5)

    hstripes.AddRelative(box, 0.8)

    main.AddRelative(hstripes,1.0)

        
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Camera Linking Tool")
    PopulateTool(t)
    t.StartSizeX = 300
    t.StartSizeY = 600
    
    if gDevelopment:
        ShowTool(t)

CreateTool()
