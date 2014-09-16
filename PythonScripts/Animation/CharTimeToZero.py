
############################################################
#
# Script for moving animation to the start of timeline(zero)
#
# ! MUST BE IN Base Layer for editing
#
# Author Sergey Solohin (Neill3d)
# homepage: http://neill3d.com
# e-mail to: s@neill3d.com
# 2012
############################################################

from pyfbsdk import *

def GetParent(model):
    parentModel = model.Parent
    if parentModel: 
        parentModel = GetParent(parentModel)
    else: 
        parentModel = model

    
    return parentModel

def MoveKeysToZero(time, node):
    fcurve = node.FCurve
    if fcurve:
        for key in fcurve.Keys:
            key.Time -= time
    
    for child in node.Nodes:
        MoveKeysToZero(time, child)


def SelectHierarchy(time, model):
    print model.Name
    MoveKeysToZero(time, model.AnimationNode)
    for child in model.Children:
        SelectHierarchy(time, child)

playerControl = FBPlayerControl()
startTime = playerControl.LoopStart
endTime = playerControl.LoopStop
zeroTime = FBTime(0)
print startTime
print zeroTime
IsZero = startTime != zeroTime
print IsZero
currChar = FBApplication().CurrentCharacter
if currChar and (startTime != zeroTime):
    hipsModel = currChar.GetModel(FBBodyNodeId.kFBHipsNodeId)
    if hipsModel:
        print hipsModel.Name
        parentModel = GetParent(hipsModel)
        if parentModel:
            SelectHierarchy(startTime, parentModel)
            
            
    playerControl.LoopStart = zeroTime
    playerControl.LoopStop = endTime - startTime
