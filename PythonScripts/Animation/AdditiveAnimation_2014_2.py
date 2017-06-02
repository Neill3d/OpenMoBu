
####################################################
# additive calculation script. Revision 2
#
# Author Sergey Solohin (Neill) 2017
#   e-mail to: s@neill3d.com
# Homepage: www.neill3d.com
# All rights reserved.
#
# Scene must have 3 takes
# where     1 - action (target)
#           2 - pose (initial)
# result goes to take 003 = action - pose
# 
##################################################

from pyfbsdk import *
import math

######################################################################
## Global Variables !

gModels = []
gInitTM = []
gResultTM = []

lSystem = FBSystem()
lApp = FBApplication()
lPlayer = FBPlayerControl()

# let character put in stance pose
lCharacter = lApp.CurrentCharacter

######################################################################
## Functions

def appendModelList( pModel ):
    
    gModels.append(pModel)
    gInitTM.append( FBMatrix() )
    gResultTM.append( FBMatrix() )
    
    for child in pModel.Children:
        appendModelList( child )


def GrabInitTM(initList):

    v = FBVector3d()
    r = FBVector3d()
    s = FBSVector(1.0, 1.0, 1.0)
    
    matrix = FBMatrix()
    
    for i in range(len(gModels)):
    
        gModels[i].GetVector(v, FBModelTransformationType.kModelTranslation, False)
        gModels[i].GetVector(r, FBModelTransformationType.kModelRotation, False)

        FBTRSToMatrix(initList[i], FBVector4d(v[0], v[1], v[2], 1.0), r, s)
    
    '''
    for model, matrix in zip(gModels, gInitTM):
        model.GetMatrix(matrix, FBModelTransformationType.kModelTransformation, False)
    '''
    
def ComputeDelta(initList, resultList):
    
    v = FBVector3d()
    r = FBVector3d()
    s = FBSVector(1.0, 1.0, 1.0)
    
    matrix = FBMatrix()
    
    for i in range(len(gModels)):
        
        gModels[i].GetVector(v, FBModelTransformationType.kModelTranslation, False)
        gModels[i].GetVector(r, FBModelTransformationType.kModelRotation, False)

        FBTRSToMatrix(matrix, FBVector4d(v[0], v[1], v[2], 1.0), r, s)
        FBGetLocalMatrix(resultList[i], initList[i], matrix)
        
    '''
    for model, initTM, resultTM in zip(gModels, gInitTM, gResultTM):

        matrix = FBMatrix()
        model.GetMatrix(matrix, FBModelTransformationType.kModelTransformation, False)
        
        FBGetLocalMatrix(resultTM, matrix, initTM)
    ''' 
    
def SetResult( pModel, matrix, pTime ):

    t = FBVector4d()
    r = FBVector3d()
    s = FBSVector()

    FBMatrixToTRS(t, r, s, matrix)

    # rotation part
    lAnimProp = pModel.Rotation
    lRotNode = lAnimProp.GetAnimationNode()
            
    if lRotNode:
        lRotNode.Nodes[0].KeyAdd( pTime, r[0] )
        lRotNode.Nodes[1].KeyAdd( pTime, r[1] )
        lRotNode.Nodes[2].KeyAdd( pTime, r[2] )

    # translation part
    lAnimProp = pModel.Translation
    lMoveNode = lAnimProp.GetAnimationNode()

    if lMoveNode:
        lMoveNode.Nodes[0].KeyAdd( pTime, t[0] )
        lMoveNode.Nodes[1].KeyAdd( pTime, t[1] )
        lMoveNode.Nodes[2].KeyAdd( pTime, t[2] )


def EvalOneFrame(lTime):
    
    lPlayer.Goto(lTime)
    
    # get initial poses
    lSystem.CurrentTake = lSystem.Scene.Takes[1]
    lSystem.Scene.Evaluate()

    GrabInitTM(gInitTM)

    # get target poses
    lSystem.CurrentTake = lSystem.Scene.Takes[0]
    lSystem.Scene.Evaluate()                
    
    ComputeDelta(gInitTM, gResultTM)

    # store result in take 3
    lSystem.CurrentTake = lSystem.Scene.Takes[2]
    lTime = FBTime()
    lTime = lSystem.LocalTime
        
    for model, resultTM in zip(gModels, gResultTM):
        
        SetResult(model, resultTM, lTime)
    

###################################################################
## MAIN

if None != lCharacter:
    lCharacter.GoToStancePose(False, True)

    startIndex = FBBodyNodeId.kFBHipsNodeId
    rootModel = lCharacter.GetModel(FBBodyNodeId(startIndex))

    # allocate storage for character models
    appendModelList(rootModel)

    lTime = FBTime()
    lStartFrame = lPlayer.LoopStart.GetFrame()
    lStopFrame = lPlayer.LoopStop.GetFrame()

    lProgress = FBProgress()
    lProgress.ProgressBegin()
    lProgress.Caption = 'Computing an additive layer'
    lProgress.Percent = 0
    
    lApp.UpdateAllWidgets()
    lApp.FlushEventQueue()
    
    for lFrame in range(lStartFrame, lStopFrame+1):

        lTime.SetTime( 0,0,0, lFrame)
        EvalOneFrame(lTime)

        if True == lProgress.UserRequestCancell():
            break
        
        p = 100.0 * lFrame / (lStopFrame-lStartFrame)
        lProgress.Percent = int(p)
        
    lProgress.ProgressDone()