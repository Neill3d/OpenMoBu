#
# script for setuping interactive HUD elements for the HUDext plugin
# Sergey <Neill3d> Solohin 2012
# e-mail to: s@neill3d.com
#   www.neill3d.com
#

# scripts works only with the the first HUD object in the scene
#
#

from pyfbsdk import *
from pyfbsdk_additions import *

region1 = FBLabel()
region2 = FBLabel()
listHUDRectSlider = FBList()
listRects = list()
region4 = FBLabel()
listHUDRectBack = FBList()
region6 = FBLabel()
conHandler = FBVisualContainer()
listModels = list()
region8 = FBButton()
region9 = FBLabel()
region10 = FBLabel()
editScriptPath = FBEdit()
buttonBuildButton = FBButton()


def ConfigurateElements():
    lScene = FBSystem().Scene

    if len(lScene.HUDs):
        HUD = lScene.HUDs[0]

        for elem in HUD.Elements:
            if elem.Is( FBHUDRectElement.TypeInfo ):
                listHUDRectSlider.Items.append(elem.Name)
                listHUDRectBack.Items.append(elem.Name)
                listRects.append(elem)

def SetupPropertyList(model):
    conHandler.Items.removeAll()
    listModels = []
    
    if model:
        listModels.append(model)
        conHandler.Items.append(model.Name)  
        print len(listModels)

def EventContainerDblClick(control, event):
    SetupPropertyList(None)
    
def EventContainerDragAndDrop(control, event):
    if event.State == FBDragAndDropState.kFBDragAndDropDrag:
        event.Accept()
    elif event.State == FBDragAndDropState.kFBDragAndDropDrop:
        SetupPropertyList( event.Components[0] )


def BuildHUDRectSlider(control, event):
    if len(listHUDRectSlider.Items) == 0 or listHUDRectSlider.ItemIndex==-1: 
        FBMessageBox( "HUDext automate", "You should assign HUD rect slider element", "Ok" )
        return
    if len(listHUDRectBack.Items) == 0 or listHUDRectBack.ItemIndex == -1: 
        FBMessageBox( "HUDext automate", "You should assign HUD rect background element", "Ok" )
        return
    #print len(listModels)
    #if len(listModels) == 0: 
    #    FBMessageBox( "HUDext automate", "You should assign slider handler element", "Ok" )
    #    return

    HUDSlider = listRects[listHUDRectSlider.ItemIndex]
    print HUDSlider.Name
    HUDBack = listRects[listHUDRectBack.ItemIndex]
    HUDBack.PositionByPercent = False
    HUDBack.ScaleByPercent = False

    camHeight = FBSystem().Scene.Cameras[0].CameraViewportHeight

    prop = HUDSlider.PropertyList.Find("Active")
    if prop == None: prop = HUDSlider.PropertyCreate("Active", FBPropertyType.kFBPT_bool, "Boolean", False, True, None)
    if prop != None: prop.Data = True
    
    prop = HUDSlider.PropertyList.Find("MinX")
    if prop == None: prop = HUDSlider.PropertyCreate("MinX", FBPropertyType.kFBPT_double, "Number", False, True, None)
    if prop != None: prop.Data = HUDBack.X
    
    prop = HUDSlider.PropertyList.Find("MaxX")
    if prop == None: prop = HUDSlider.PropertyCreate("MaxX", FBPropertyType.kFBPT_double, "Number", False, True, None)
    if prop != None: prop.Data = HUDBack.X + HUDBack.Width
    
    prop = HUDSlider.PropertyList.Find("MinY")
    if prop == None: prop = HUDSlider.PropertyCreate("MinY", FBPropertyType.kFBPT_double, "Number", False, True, None)
    if prop != None: prop.Data = HUDBack.Y
    
    prop = HUDSlider.PropertyList.Find("MaxY")
    if prop == None: prop = HUDSlider.PropertyCreate("MaxY", FBPropertyType.kFBPT_double, "Number", False, True, None)
    if prop != None: prop.Data = HUDBack.Y + HUDBack.Height
    
    #conModel = listModels[0]
    #if conModel != None:
    #    prop = HUDSlider.PropertyAddReferenceProperty(conModel.Translation)
    
def BuildHUDTextScript(control, event):
    
    lScene = FBSystem().Scene

    if len(lScene.HUDs):
        HUD = lScene.HUDs[0]
        
        HUDTextElement = FBHUDTextElement("Script button")
        #Scene.ConnectSrc(HUD)          # Connect the HUD to the scene
        HUDTextElement.Content = "Script button"
        HUDTextElement.X = 0
        HUDTextElement.Y = 0
        HUDTextElement.Scale = 2
        prop = HUDTextElement.PropertyCreate('Active', FBPropertyType.kFBPT_bool, 'Boolean', False, True, None) 
        if prop != None: prop.Data = True
        prop = HUDTextElement.PropertyCreate("ScriptExecute", FBPropertyType.kFBPT_charptr, "String", False, True, None) 
        if prop != None: prop.Data = editScriptPath.Text
        HUD.ConnectSrc(HUDTextElement) # Connect HUDTextElement to the HUD

def PopulateTool(t):
    #populate regions here

    x = FBAddRegionParam(63,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(10,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(165,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region1","region1", x, y, w, h)

    t.SetControl("region1", region1)
    region1.Visible = True
    region1.ReadOnly = False
    region1.Enabled = True
    region1.Hint = ""
    region1.Caption = "Rect slider creation"
    region1.Style = FBTextStyle.kFBTextStyleBold
    region1.Justify = FBTextJustify.kFBTextJustifyLeft
    region1.WordWrap = True
    
    x = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(115,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region2","region2", x, y, w, h)

    t.SetControl("region2", region2)
    region2.Visible = True
    region2.ReadOnly = False
    region2.Enabled = True
    region2.Hint = ""
    region2.Caption = "HUD Rect slider"
    region2.Style = FBTextStyle.kFBTextStyleNone
    region2.Justify = FBTextJustify.kFBTextJustifyLeft
    region2.WordWrap = True
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(40,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(115,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("listHUDRectSlider","listHUDRectSlider", x, y, w, h)

    t.SetControl("listHUDRectSlider", listHUDRectSlider)
    listHUDRectSlider.Visible = True
    listHUDRectSlider.ReadOnly = False
    listHUDRectSlider.Enabled = True
    listHUDRectSlider.Hint = ""
    listHUDRectSlider.ItemIndex = -1
    listHUDRectSlider.MultiSelect = False
    listHUDRectSlider.ExtendedSelect = False
    listHUDRectSlider.Style = FBListStyle.kFBDropDownList
    
    x = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(70,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(115,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region4","region4", x, y, w, h)

    t.SetControl("region4", region4)
    region4.Visible = True
    region4.ReadOnly = False
    region4.Enabled = True
    region4.Hint = ""
    region4.Caption = "HUD Rect background"
    region4.Style = FBTextStyle.kFBTextStyleNone
    region4.Justify = FBTextJustify.kFBTextJustifyLeft
    region4.WordWrap = True
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(70,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(115,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("listHUDRectBack","listHUDRectBack", x, y, w, h)

    t.SetControl("listHUDRectBack", listHUDRectBack)
    listHUDRectBack.Visible = True
    listHUDRectBack.ReadOnly = False
    listHUDRectBack.Enabled = True
    listHUDRectBack.Hint = ""
    listHUDRectBack.ItemIndex = -1
    listHUDRectBack.MultiSelect = False
    listHUDRectBack.ExtendedSelect = False
    listHUDRectBack.Style = FBListStyle.kFBDropDownList
    
    x = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(100,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(115,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region6","region6", x, y, w, h)

    t.SetControl("region6", region6)
    region6.Visible = True
    region6.ReadOnly = False
    region6.Enabled = True
    region6.Hint = ""
    region6.Caption = "Slider handler element"
    region6.Style = FBTextStyle.kFBTextStyleNone
    region6.Justify = FBTextJustify.kFBTextJustifyLeft
    region6.WordWrap = True
    
    x = FBAddRegionParam(150,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(100,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(115,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("conHandler","conHandler", x, y, w, h)

    t.SetControl("conHandler", conHandler)
    conHandler.Visible = True
    conHandler.ReadOnly = False
    conHandler.Enabled = True
    conHandler.Hint = ""
    conHandler.ItemIndex = -1
    conHandler.ItemWrap = False
    conHandler.IconPosition = FBIconPosition.kFBIconLeft
    conHandler.ItemWidth = 32
    conHandler.ItemHeight = 32
    conHandler.OnDragAndDrop.Add(EventContainerDragAndDrop)
    conHandler.OnDblClick.Add(EventContainerDblClick)
    
    x = FBAddRegionParam(65,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(135,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(135,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region8","region8", x, y, w, h)

    t.SetControl("region8", region8)
    region8.Visible = True
    region8.ReadOnly = False
    region8.Enabled = True
    region8.Hint = ""
    region8.Caption = "Build / Update"
    region8.State = 0
    region8.Style = FBButtonStyle.kFBPushButton
    region8.Justify = FBTextJustify.kFBTextJustifyCenter
    region8.Look = FBButtonLook.kFBLookNormal
    region8.OnClick.Add( BuildHUDRectSlider )
    
    x = FBAddRegionParam(65,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(185,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(175,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region9","region9", x, y, w, h)

    t.SetControl("region9", region9)
    region9.Visible = True
    region9.ReadOnly = False
    region9.Enabled = True
    region9.Hint = ""
    region9.Caption = "HUD script button creation"
    region9.Style = FBTextStyle.kFBTextStyleBold
    region9.Justify = FBTextJustify.kFBTextJustifyCenter
    region9.WordWrap = True
    
    x = FBAddRegionParam(20,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(215,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(95,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region10","region10", x, y, w, h)

    t.SetControl("region10", region10)
    region10.Visible = True
    region10.ReadOnly = False
    region10.Enabled = True
    region10.Hint = ""
    region10.Caption = "Enter script path:"
    region10.Style = FBTextStyle.kFBTextStyleNone
    region10.Justify = FBTextJustify.kFBTextJustifyLeft
    region10.WordWrap = True
    
    x = FBAddRegionParam(125,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(215,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(185,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editScriptPath","editScriptPath", x, y, w, h)

    t.SetControl("editScriptPath", editScriptPath)
    editScriptPath.Visible = True
    editScriptPath.ReadOnly = False
    editScriptPath.Enabled = True
    editScriptPath.Hint = ""
    editScriptPath.Text = "C:\\"
    editScriptPath.PasswordMode = False
    
    x = FBAddRegionParam(65,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(255,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(135,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonBuildButton","buttonBuildButton", x, y, w, h)

    t.SetControl("buttonBuildButton", buttonBuildButton)
    buttonBuildButton.Visible = True
    buttonBuildButton.ReadOnly = False
    buttonBuildButton.Enabled = True
    buttonBuildButton.Hint = ""
    buttonBuildButton.Caption = "Build"
    buttonBuildButton.State = 0
    buttonBuildButton.Style = FBButtonStyle.kFBPushButton
    buttonBuildButton.Justify = FBTextJustify.kFBTextJustifyCenter
    buttonBuildButton.Look = FBButtonLook.kFBLookNormal
    buttonBuildButton.OnClick.Add(BuildHUDTextScript)
    
def CreateTool():
    t = FBCreateUniqueTool("HUDext setup automate (Sergey <Neill3d> Solohin)")
    t.StartSizeX = 400
    t.StartSizeY = 400
    PopulateTool(t)
    ConfigurateElements()
    ShowTool(t)
CreateTool()
