
#########################################################
##
## Global Post Processing Settings dialog box
##
## This script is an addon to the post processing manager plugin
##  
##
## Author Sergey Solohin (Neill3d) 2018
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
##  Special for les Androids Associes
##
#########################################################

from pyfbsdk import *
from pyfbsdk_additions import *

TOOL_TITLE = 'Post Processing Tool'
gDEVELOPMENT = True
DATA_CLASSNAME = 'PostPersistentData' # PostPersistentData
DATA_DEFAULT_NAME = 'Post Processing'
DATA_CLASSID = 55

lApp = FBApplication()
lSystem = FBSystem()

lListObjects = FBList()
lTabPanel = FBTabPanel()
lLayoutMain = FBLayout()
lScrollBox = FBScrollBox()

lLayoutCommon = FBLayout()
lLayoutDOF = FBLayout()
lLayoutSSAO = FBLayout()
lLayoutMotionBlur = FBLayout()
lLayoutColor = FBLayout()
lLayoutLensFlare = FBLayout()
lLayoutDisplacement = FBLayout()
lLayoutFishEye = FBLayout()
lLayoutFilmGrain = FBLayout()
lLayoutVignetting = FBLayout()

lSettings = None

global propsCommon
global gEffectProps
global gEffectHeight

regionx = 50
regionw = 300

commonNames = ["Active", "Use Camera Object", "Camera", "SSAO", "Motion Blur", "Depth Of Field", "Color Correction", "Lens Flare", "Displacement", "Fish Eye", "Film Grain", "Vignetting"]

gEffectNames = []

def UpdateDataList():
    
    itemIndex = lListObjects.ItemIndex
    if itemIndex < 0:
        itemIndex = 0
    lListObjects.Items.removeAll()
    
    for obj in lSystem.Scene.UserObjects:
        if obj.ClassName() == DATA_CLASSNAME:
            print obj.Name
            lListObjects.Items.append(obj.LongName)
    
    if itemIndex < len(lListObjects.Items):
        lListObjects.ItemIndex = itemIndex

    print len(lListObjects.Items)

def UpdateCurrentData():
    global lSettings
    
    lSettings = None
    if lListObjects.ItemIndex >= 0:
        name = lListObjects.Items[lListObjects.ItemIndex]
        print name
        for obj in lSystem.Scene.UserObjects:
            if obj.ClassName() == DATA_CLASSNAME and name == obj.LongName:
                lSettings = obj
                print name
    return lSettings

def CreateANewData():
    
    # if nothing has found, then create a new one
    obj = FBCreateObject( 'FbxStorable/User', DATA_CLASSNAME, DATA_DEFAULT_NAME )
    return obj

def PrepNames(commonNames, firstEffectIndex, numberOfEffects):
    
    temp = CreateANewData()
    
    allTheNames = []
    
    if temp is not None:
        lastEffectIndex = firstEffectIndex + numberOfEffects
        for i in range(firstEffectIndex, lastEffectIndex):
            
            effectNames = []
            
            effectName = commonNames[i]

            lastprop = None
            if i < (lastEffectIndex-1):
                lastprop = temp.PropertyList.Find(commonNames[i+1])
            
            firstprop = temp.PropertyList.Find(effectName)

            
            startAdding = False
            for prop in temp.PropertyList:
                
                if prop == firstprop:
                    startAdding = True
                elif prop == lastprop:
                    break
        
                if True == startAdding:
                    effectNames.append(prop.GetName())
                
            allTheNames.append(effectNames)
        
        temp.FBDelete()
        del(temp)
    
    return allTheNames

def CreateEffectUI(lyt, names):
    
    props = []
    height = 20
   
    # Create Main region frame:
    x = FBAddRegionParam(regionx,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(regionw,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    lyt.AddRegion("main","main", x, y, w, h)
    lyt.SetControl("main",main)
    
    for i, name in enumerate(names):
        
        b = FBEditProperty()
        b.Caption = name
        
        lProp = None
        if lSettings is not None:
            lProp = lSettings.PropertyList.Find(name)
        b.Property = lProp
        
        main.Add(b, 25)
        props.append(b)
        
        height += 30

    return [props, height]

def CloseCurrentDataUI():
    
    for prop in propsCommon: prop.Property = None
    
    for l in gEffectProps:
        for prop in l: prop.Property = None
    
def RefreshCurrentDataUI():
    if lSettings is not None:

        for i, name in enumerate(commonNames):
            lProp = lSettings.PropertyList.Find(name)
            propsCommon[i].Property = lProp
            
        for listnames, listprops in zip(gEffectNames, gEffectProps):
            
            for name, prop in zip(listnames, listprops):
                lProp = lSettings.PropertyList.Find(name)
                prop.Property = lProp
        
def PrepTabLayout(lyt):
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    lyt.AddRegion("main","main", x, y, w, h)
    
def PrepScrollBox(scroll):
    
    x = FBAddRegionParam(0,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(0,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(0,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(0,FBAttachType.kFBAttachBottom,"")
    
    # Content property is the scrollbox's layout: create a region in it
    scroll.Content.AddRegion( "content", "content", x, y, w, h )
    
def ChooseLayoutForScroll(scroll, index):
    
    if index == 0:
        scroll.Content.SetControl("content", lLayoutCommon)
    elif index == 1:
        scroll.Content.SetControl("content", lLayoutSSAO)
    elif index == 2:
        scroll.Content.SetControl("content", lLayoutMotionBlur)
    elif index == 3:
        scroll.Content.SetControl("content", lLayoutDOF)
    elif index == 4:
        scroll.Content.SetControl("content", lLayoutColor)
    elif index == 5:
        scroll.Content.SetControl("content", lLayoutLensFlare)
    elif index == 6:
        scroll.Content.SetControl("content", lLayoutDisplacement)
    elif index == 7:
        scroll.Content.SetControl("content", lLayoutFishEye)
    elif index == 8:
        scroll.Content.SetControl("content", lLayoutFilmGrain)
    elif index == 9:
        scroll.Content.SetControl("content", lLayoutVignetting)

    scroll.SetContentSize(400, gEffectHeight[index])
    
def ChooseLayout(lyt, index):
    
    if index == 0:
        lyt.SetControl("main", lLayoutCommon)
    elif index == 1:
        lyt.SetControl("main", lLayoutSSAO)
    elif index == 2:
        lyt.SetControl("main", lLayoutMotionBlur)        
    elif index == 3:
        lyt.SetControl("main", lLayoutDOF)
    elif index == 4:
        lyt.SetControl("main", lLayoutColor)
    elif index == 5:
        lyt.SetControl("main", lLayoutLensFlare)
    elif index == 6:
        lyt.SetControl("main", lLayoutDisplacement)
    elif index == 7:
        lyt.SetControl("main", lLayoutFishEye)
    elif index == 8:
        lyt.SetControl("main", lLayoutFilmGrain)
    elif index == 9:
        lyt.SetControl("main", lLayoutVignetting)

def OnCurrentDataChange(control, event):
    UpdateCurrentData()
    RefreshCurrentDataUI()

def OnTabChange(control, event):
    
    ChooseLayoutForScroll( lScrollBox, lTabPanel.ItemIndex )

def OnButtonReset(control, event):
    
    if lSettings is not None:
        lProp = lSettings.PropertyList.Find("Reset To Default")
        if lProp:
            lProp()

def OnButtonSelect(control, event):
    
    global lSettings
    lSettings = GetGlobalSettingsObject()
    
    if lSettings is not None:
        lSettings.Selected = True

def OnButtonRefresh(control, event):
    
    UpdateDataList()
    UpdateCurrentData()
    RefreshCurrentDataUI()

global gNewNode

def OnUIIdleCheckNewNode(control, event):

    lSystem.OnUIIdle.Remove(OnUIIdleCheckNewNode)

    if gNewNode is not None:

        prop = gNewNode.PropertyList.Find('UniqueClassId')
        if prop is not None and DATA_CLASSID == prop.Data:
            print 'we have found our node'
            # user has been added a new post processing
            UpdateDataList()
            # make new item current
            lListObjects.ItemIndex = len(lListObjects.Items)-1
            
            UpdateCurrentData()
            RefreshCurrentDataUI()
    

def OnSceneChange(control, event):
    
    #FBEventSceneChange  lEvent

    global gNewNode
    eventType = event.Type
    
    if event.Component is not None and event.ChildComponent is not None:
        
        
        if lSystem.Scene == event.Component:
            if eventType == FBSceneChangeType.kFBSceneChangeChangedParent or eventType == FBSceneChangeType.kFBSceneChangeAttach:
                gNewNode = event.ChildComponent
                lSystem.OnUIIdle.Add(OnUIIdleCheckNewNode)

            elif eventType == FBSceneChangeType.kFBSceneChangeDetach:
                
                prop = event.ChildComponent.PropertyList.Find('UniqueClassId')
                if prop is not None and DATA_CLASSID == prop.Data:
                    print 'deleted by a user'
                    # user has been deleted a new post processing
                    CloseCurrentDataUI()
                    
                    UpdateDataList()
                    UpdateCurrentData()
                    
                    RefreshCurrentDataUI()

    if eventType == FBSceneChangeType.kFBSceneChangeRenamed or eventType == FBSceneChangeType.kFBSceneChangeRenamedPrefix:
        
        prop = event.Component.PropertyList.Find('UniqueClassId')
        if prop is not None and DATA_CLASSID == prop.Data:
            UpdateDataList()
            #UpdateCurrentData()
                    
            #RefreshCurrentDataUI()

def CloseConnection(control, event):
    global lSettings
    lSettings = None
    CloseCurrentDataUI()
    lListObjects.Items.removeAll()

def Bind():
    
    UpdateDataList()
    UpdateCurrentData()
    RefreshCurrentDataUI()
    
    lApp.OnFileNew.Add(CloseConnection)
    lApp.OnFileOpen.Add(CloseConnection)
    lApp.OnFileMerge.Add(CloseConnection)
    lApp.OnFileOpenCompleted.Add(OnButtonRefresh)

    lSystem.Scene.OnChange.Add(OnSceneChange)

def UnBind():
    
    global lSettings
    lSettings = None
    CloseCurrentDataUI()
    lListObjects.Items.removeAll()
    
    lApp.OnFileNew.Remove(CloseConnection)
    lApp.OnFileOpen.Remove(CloseConnection)
    lApp.OnFileMerge.Remove(CloseConnection)
    lApp.OnFileOpenCompleted.Remove(OnButtonRefresh)

    lSystem.Scene.OnChange.Remove(OnSceneChange)
    
def OnUnbind(control, event):
    print "***** ", control, " has been Unbound *****"
    UnBind()
    
def OnButtonAddClick(control, event):
    CreateANewData()
    UpdateDataList()
    # make new item current
    lListObjects.ItemIndex = len(lListObjects.Items)-1
    
    UpdateCurrentData()
    RefreshCurrentDataUI()
    
def OnButtonRemoveClick(control, event):
    global lSettings
    userChoise = FBMessageBox( TOOL_TITLE, 'Are you sure you want to remove?', 'Ok', 'Cancel')
    if 1 == userChoise:
        if lSettings is not None:
            lSettings.PropertyList.Find('Active').Data = False
            
            CloseCurrentDataUI()
            lSettings.FBDelete()
            
            UpdateDataList()
            UpdateCurrentData()
            
            RefreshCurrentDataUI()

def OnButtonRenameClick(control, event):
    
    if lListObjects.ItemIndex >= 0 and lSettings is not None:

        name = lListObjects.Items[lListObjects.ItemIndex]
    
        userChoise = FBMessageBoxGetUserValue(TOOL_TITLE, 'Please enter a new name', name, FBPopupInputType.kFBPopupString, 'Ok', 'Cancel')
        if 1 == userChoise[0]:

            lSettings.LongName = userChoise[1]
            
            UpdateDataList()
            UpdateCurrentData()


def OnButtonDuplicateClick(control, event):
    
    if lSettings is not None:
        
        lPrevSettings = lSettings
        
        CreateANewData()
        UpdateDataList()
        # make new item current
        lListObjects.ItemIndex = len(lListObjects.Items)-1
        
        UpdateCurrentData()
        RefreshCurrentDataUI()
    
        # copy data
        startCopy = False
        
        if lSettings is not None:
            for lDstProp, lSrcProp in zip(lSettings.PropertyList, lPrevSettings.PropertyList):
                
                if lDstProp.GetName() == 'Active':
                    startCopy = True
                    
                if True == startCopy:                    
                    propType = lDstProp.GetPropertyType()
                    
                    if FBPropertyType.kFBPT_int == propType or FBPropertyType.kFBPT_bool == propType:
                        value = lSrcProp.Data
                        lDstProp.Data = value
                    elif FBPropertyType.kFBPT_double == propType:
                        value = lSrcProp.Data
                        lDstProp.Data = value
                    elif FBPropertyType.kFBPT_ColorRGB == propType or FBPropertyType.kFBPT_ColorRGBA == propType:
                        value = lSrcProp.Data
                        lDstProp.Data = value
    
    else:
        FBMessageBox( TOOL_TITLE, 'There is no current data to duplicate', 'Ok' )
    
def OnButtonSelectClick(control, event):

    if lSettings is not None:
        lSettings.Selected = True


def OnButtonAbout(control, event):
    
    FBMessageBox( TOOL_TITLE, "Sergey <Neill3d> Solokhin (c) 2018\nSpecial For Les Androids Associes", "Ok" )

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
    
    # objects panel
    
    mainpanel = FBHBoxLayout()
    
    b = FBLabel()
    b.Caption = 'Current Data:'
    mainpanel.Add(b, 80)
    
    lListObjects.Style = FBListStyle.kFBDropDownList
    lListObjects.OnChange.Add( OnCurrentDataChange )
    UpdateDataList()
    UpdateCurrentData()
    mainpanel.Add(lListObjects, 180)
    
    b = FBLabel()
    b.Caption = ''
    mainpanel.Add(b, 15)
    
    b = FBButton()
    b.Caption = 'Add'
    b.OnClick.Add( OnButtonAddClick )
    mainpanel.Add(b, 50)
    
    b = FBButton()
    b.Caption = 'Remove'
    b.OnClick.Add( OnButtonRemoveClick )
    mainpanel.Add(b, 50)

    b = FBButton()
    b.Caption = 'Rename'
    b.OnClick.Add( OnButtonRenameClick )
    mainpanel.Add(b, 50)
    
    b = FBButton()
    b.Caption = 'Duplicate'
    b.OnClick.Add( OnButtonDuplicateClick )
    mainpanel.Add(b, 50)
    
    b = FBButton()
    b.Caption = 'Select'
    b.OnClick.Add( OnButtonSelectClick )
    mainpanel.Add(b, 50)
    
    
    hstripes.Add(mainpanel, 25)
    
    b = FBLabel()
    b.Caption = ''
    hstripes.Add(b, 5)
    
    #
    items = ["Common", "SSAO", "Motion Blur", "Depth Of Field", "Color Correction", "Lens Flare", "Displacement", "Fish Eye", "Film Grain", "Vignetting"]
    for item in items:
        lTabPanel.Items.append(item)
    lTabPanel.ItemIndex = 0
    lTabPanel.OnChange.Add( OnTabChange )
    hstripes.Add(lTabPanel, 30)
    
    #
    global propsCommon
    global gEffectProps, gEffectNames, gEffectHeight

    gEffectHeight = []

    firstEffectIndex = 3
    numberOfEffects = 9

    gEffectNames = PrepNames(commonNames, firstEffectIndex, numberOfEffects)
        
    propsCommon, height = CreateEffectUI(lLayoutCommon, commonNames)
    gEffectHeight.append(height)
    
    layouts = [lLayoutSSAO, lLayoutMotionBlur, lLayoutDOF, lLayoutColor, lLayoutLensFlare, lLayoutDisplacement, lLayoutFishEye, lLayoutFilmGrain, lLayoutVignetting]
    gEffectProps = []
    for effectNames, layout in zip(gEffectNames, layouts):
        
        props,height = CreateEffectUI(layout, effectNames)
        
        gEffectProps.append(props)
        gEffectHeight.append(height)
                    
    #
    lScrollBox.SetContentSize(400, 800)
    
    PrepTabLayout(lLayoutMain)
    PrepScrollBox(lScrollBox)
    ChooseLayoutForScroll(lScrollBox, 0)
    #hstripes.AddRelative(lLayoutMain, 0.8)
    hstripes.AddRelative(lScrollBox, 0.8)
    
    

    #
    box = FBHBoxLayout()

    b = FBButton()
    b.Caption = "Reset To Default"
    b.OnClick.Add( OnButtonReset )
    box.Add(b, 120)

    b = FBButton()
    b.Caption = "Refresh"
    b.OnClick.Add( OnButtonRefresh )
    box.Add(b, 80)

    
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add( OnButtonAbout )
    box.Add(b, 80)
    
    hstripes.Add(box, 30)

    main.AddRelative(hstripes,1.0)

def EventShowTool(control, event):
    print "shown"
    if True == event.Shown:
        Bind()
    else:
        UnBind()

def CreateTool():

    global t    
    global lSettings
    #lSettings = GetGlobalSettingsObject()
      
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool(TOOL_TITLE)
    PopulateTool(t)
    #Bind()
    #t.OnUnbind.Add(OnUnbind)
    t.OnShow.Add( EventShowTool )
    t.StartSizeX = 700
    t.StartSizeY = 520

    if gDEVELOPMENT:
        ShowTool(t)


CreateTool()