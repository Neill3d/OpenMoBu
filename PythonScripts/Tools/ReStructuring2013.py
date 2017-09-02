
#
# Script for keeping objects transform animation while changing heirarchy
#
#  Version for MotionBuilder 2013
#
# Sergey Solohin (Neill3d) 2013
#   e-mail to: s@neill3d.com
#  www.neill3d.com
#

from pyfbsdk import *
from pyfbsdk_additions import *

gSystem = FBSystem()

source = []
nulls = []
constraints = []

# prepare constraint type
lPosIdx = -1

lMgr = FBConstraintManager()
for lIdx in range( lMgr.TypeGetCount() ):
    if lMgr.TypeGetName( lIdx ) == 'Parent/Child':
        lPosIdx = lIdx
        

def EventButtonHold(control, event):
    
    global source
    global nulls
    global constraints
    
    source[:] = []
    nulls[:] = []
    constraints[:] = []
    
    mList = FBModelList()
    
    FBGetSelectedModels(mList)
    
    for obj in mList:
        newObj = FBModelNull( obj.Name + '_Null' )
        newObj.Show = True
        newObj.Selected = True
        obj.Selected = False
        
        source.append(obj.Name)
        nulls.append(newObj)
        
        if lPosIdx >= 0:
            lConstraint = lMgr.TypeCreateConstraint(lPosIdx)
            lConstraint.ReferenceAdd(0, newObj)
            lConstraint.ReferenceAdd(1, obj)    
            lConstraint.Active = True
                
            constraints.append(lConstraint)
            
        
    # plot all new nulls    
    take = gSystem.CurrentTake
    lPeriod = FBTime(0,0,0,1)
    if take: take.PlotTakeOnSelected(lPeriod)
            
    # switch off the constraints            
    for lConstraint in constraints:
        lConstraint.Active = False
        
        objA = lConstraint.ReferenceGet(0, 0)
        objB = lConstraint.ReferenceGet(1, 0)
        
        lConstraint.ReferenceRemove(0, objA)
        lConstraint.ReferenceRemove(1, objB)
                
        lConstraint.ReferenceAdd(0, objB)
        lConstraint.ReferenceAdd(1, objA)
        
    
def EventButtonFetch(control, event):
    print "fetch"

    # activate constraints
    i=0
    for lConstraint in constraints:
        lConstraint.Active = True
        
        objA = lConstraint.ReferenceGet(0,0)
        if objA == None:
            objA = FBFindModelByLabelName(source[i])
            lConstraint.ReferenceAdd(0,objA)
            
        if objA != None: objA.Selected = True
        
        i = i + 1

    # plot animations
    take = gSystem.CurrentTake
    lPeriod = FBTime(0,0,0,1)
    if take: take.PlotTakeOnSelected(lPeriod)
    
    # remove unneeded objects and constraints
    for lConstraint in constraints:
        lConstraint.Active = False
        
        objB = lConstraint.ReferenceGet(1,0)
        
        objB.FBDelete()
        lConstraint.FBDelete()


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
    b.Caption = "Fetch animation"
    b.OnClick.Add(EventButtonFetch)
    main.AddRegion("fetch", "fetch", x, y, w, h)
    main.SetControl("fetch", b)
    
def CreateTool():    
    # Tool creation will serve as the hub for all other controls
    t = FBCreateUniqueTool("Restructuring (Neill3d)")
    PopulateLayout(t)
    t.StartSizeX = 400
    t.StartSizeY = 220
    ShowTool(t)


CreateTool()





