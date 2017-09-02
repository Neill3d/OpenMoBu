####################################################################
##
## StayOnFloor.py
## Author Sergey <Neill3d> Solohin 2013
## s@neill3d.com
## www.neill3d.com
##
####################################################################

from pyfbsdk import *
from pyfbsdk_additions import *

region1 = FBLabel()
editStart = FBEditNumber()
buttonStart = FBButton()
region4 = FBLabel()
editStop = FBEditNumber()
buttonStop = FBButton()
buttonTimeline = FBButton()
buttonAction = FBButton()
listAction = FBList()
region9 = FBLabel()
editBlendTime = FBEditNumber()

def PopulateTool(t):
    #populate regions here

    x = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region1","region1", x, y, w, h)

    t.SetControl("region1", region1)
    region1.Visible = True
    region1.ReadOnly = False
    region1.Enabled = True
    region1.Hint = ""
    region1.Caption = "Start"
    region1.Style = FBTextStyle.kFBTextStyleNone
    region1.Justify = FBTextJustify.kFBTextJustifyLeft
    region1.WordWrap = True
    
    x = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(45,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editStart","editStart", x, y, w, h)

    t.SetControl("editStart", editStart)
    editStart.Visible = True
    editStart.ReadOnly = False
    editStart.Enabled = True
    editStart.Hint = ""
    editStart.Value = 0.000000
    editStart.Min = 0.000000
    editStart.Max = 0.000000
    editStart.Precision = 0.000000
    editStart.LargeStep = 5.000000
    editStart.SmallStep = 1.000000
    
    x = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(75,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonStart","buttonStart", x, y, w, h)

    t.SetControl("buttonStart", buttonStart)
    buttonStart.Visible = True
    buttonStart.ReadOnly = False
    buttonStart.Enabled = True
    buttonStart.Hint = ""
    buttonStart.Caption = "Assign Start"
    buttonStart.State = 0
    buttonStart.Style = FBButtonStyle.kFBPushButton
    buttonStart.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonStart.Look = FBButtonLook.kFBLookNormal
    buttonStart.OnClick.Add(ButtonStartEvent)
    
    x = FBAddRegionParam(110,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region4","region4", x, y, w, h)

    t.SetControl("region4", region4)
    region4.Visible = True
    region4.ReadOnly = False
    region4.Enabled = True
    region4.Hint = ""
    region4.Caption = "Stop"
    region4.Style = FBTextStyle.kFBTextStyleNone
    region4.Justify = FBTextJustify.kFBTextJustifyLeft
    region4.WordWrap = True
    
    x = FBAddRegionParam(110,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(45,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editStop","editStop", x, y, w, h)

    t.SetControl("editStop", editStop)
    editStop.Visible = True
    editStop.ReadOnly = False
    editStop.Enabled = True
    editStop.Hint = ""
    editStop.Value = 0.000000
    editStop.Min = 0.000000
    editStop.Max = 0.000000
    editStop.Precision = 0.000000
    editStop.LargeStep = 5.000000
    editStop.SmallStep = 1.000000
    
    x = FBAddRegionParam(110,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(75,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonStop","buttonStop", x, y, w, h)

    t.SetControl("buttonStop", buttonStop)
    buttonStop.Visible = True
    buttonStop.ReadOnly = False
    buttonStop.Enabled = True
    buttonStop.Hint = ""
    buttonStop.Caption = "Assign Stop"
    buttonStop.State = 0
    buttonStop.Style = FBButtonStyle.kFBPushButton
    buttonStop.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonStop.Look = FBButtonLook.kFBLookNormal
    buttonStop.OnClick.Add(ButtonStopEvent)
    
    x = FBAddRegionParam(205,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(75,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonTimeline","buttonTimeline", x, y, w, h)

    t.SetControl("buttonTimeline", buttonTimeline)
    buttonTimeline.Visible = True
    buttonTimeline.ReadOnly = False
    buttonTimeline.Enabled = True
    buttonTimeline.Hint = ""
    buttonTimeline.Caption = "Timeline range"
    buttonTimeline.State = 0
    buttonTimeline.Style = FBButtonStyle.kFBPushButton
    buttonTimeline.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonTimeline.Look = FBButtonLook.kFBLookNormal
    buttonTimeline.OnClick.Add(ButtonTimelineEvent)
    
    x = FBAddRegionParam(45,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(120,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(110,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("buttonAction","buttonAction", x, y, w, h)

    t.SetControl("buttonAction", buttonAction)
    buttonAction.Visible = True
    buttonAction.ReadOnly = False
    buttonAction.Enabled = True
    buttonAction.Hint = ""
    buttonAction.Caption = "Do it!"
    buttonAction.State = 0
    buttonAction.Style = FBButtonStyle.kFBPushButton
    buttonAction.Justify = FBTextJustify.kFBTextJustifyLeft
    buttonAction.Look = FBButtonLook.kFBLookNormal
    buttonAction.OnClick.Add(ButtonActionEvent)
    
    x = FBAddRegionParam(175,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(120,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(110,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(30,FBAttachType.kFBAttachNone,"")
    t.AddRegion("listAction","listAction", x, y, w, h)

    t.SetControl("listAction", listAction)
    listAction.Style = FBListStyle.kFBDropDownList
    listAction.Items.append('T')
    listAction.Items.append('R')
    listAction.Items.append('TR')
    listAction.Items.append('S')
    listAction.Items.append('TRS')
    listAction.ItemIndex = 2
    
    x = FBAddRegionParam(205,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(15,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("region9","region9", x, y, w, h)

    t.SetControl("region9", region9)
    region9.Visible = True
    region9.ReadOnly = False
    region9.Enabled = True
    region9.Hint = ""
    region9.Caption = "Blend Time"
    region9.Style = FBTextStyle.kFBTextStyleNone
    region9.Justify = FBTextJustify.kFBTextJustifyLeft
    region9.WordWrap = True
    
    x = FBAddRegionParam(205,FBAttachType.kFBAttachNone,"")
    y = FBAddRegionParam(45,FBAttachType.kFBAttachNone,"")
    w = FBAddRegionParam(80,FBAttachType.kFBAttachNone,"")
    h = FBAddRegionParam(25,FBAttachType.kFBAttachNone,"")
    t.AddRegion("editBlendTime","editBlendTime", x, y, w, h)

    t.SetControl("editBlendTime", editBlendTime)
    editBlendTime.Visible = True
    editBlendTime.ReadOnly = False
    editBlendTime.Enabled = True
    editBlendTime.Hint = ""
    editBlendTime.Value = 3.000000
    editBlendTime.Min = 0.000000
    editBlendTime.Max = 100.000000
    editBlendTime.Precision = 0.000000
    editBlendTime.LargeStep = 1.000000
    editBlendTime.SmallStep = 0.100000
    
def ButtonStartEvent(control, event):
    time = FBSystem().LocalTime
    frame = time.GetFrame()
    editStart.Value = frame
    
def ButtonStopEvent(control, event):
    time = FBSystem().LocalTime
    frame = time.GetFrame()
    editStop.Value = frame        
    
def ButtonTimelineEvent(control, event):
    btime = FBPlayerControl().LoopStart
    etime = FBPlayerControl().LoopStop
    editStart.Value = btime.GetFrame()
    editStop.Value = etime.GetFrame()
    
def IsTimeInside(timespan, time):
    if (time > timespan.GetStart() and time < timespan.GetStop() ): return True
    return False
    
def ProcessFCurve(curve, currTime, startTime, stopTime, blendTime):
    value = curve.Evaluate(currTime)
    count = len(curve.Keys)
    
    startTimeBlend = startTime - blendTime
    stopTimeBlend = stopTime + blendTime
    
    if curve: curve.KeyDeleteByTimeRange(startTimeBlend, stopTimeBlend, True)
    
    startNdx = curve.KeyAdd(startTime, value)
    stopNdx = curve.KeyAdd(stopTime, value)
    
    if startNdx >= 0: 
        curve.Keys[startNdx].TangentMode = FBTangentMode.kFBTangentModeUser
        curve.Keys[startNdx].LeftDerivative = 0.0
        curve.Keys[startNdx].RightDerivative = 0.0

    if stopNdx >= 0: 
        curve.Keys[stopNdx].TangentMode = FBTangentMode.kFBTangentModeUser   
        curve.Keys[stopNdx].LeftDerivative = 0.0
        curve.Keys[stopNdx].RightDerivative = 0.0
    
def ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime):
    for node in animNode.Nodes:
        if node.FCurve:
            ProcessFCurve(node.FCurve, currTime, startTime, stopTime, blendTime)
    
def ButtonActionEvent(control, event):
    currTime = FBSystem().LocalTime
    startTime = FBTime(0,0,0, int(editStart.Value) )
    stopTime = FBTime(0,0,0, int(editStop.Value) )
    blendTime = FBTime(0,0,0, int(editBlendTime.Value) )
    
    
    models = FBModelList()
    FBGetSelectedModels(models)
    
    mode = listAction.ItemIndex
    
    for model in models:
        if mode in [0,2,4]:
            animNode = model.Translation.GetAnimationNode()
            if animNode: ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime)
        if mode in [1,2,4]:
            animNode = model.Rotation.GetAnimationNode()
            if animNode: ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime)
        if mode in [3,4]:
            animNode = model.Scaling.GetAnimationNode()
            if animNode: ProcessAnimationNode(animNode, currTime, startTime, stopTime, blendTime)
    
def CreateTool():
    t = FBCreateUniqueTool("Stay on floor (Sergey <Neill3d> Solokhin 2013)")
    t.StartSizeX = 325
    t.StartSizeY = 200
    PopulateTool(t)
    ShowTool(t)
CreateTool()
