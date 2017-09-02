

####################################################
#
#   AdditiveLayer_IntoNewTake.py
#
# Extract Animation Layer into a new take
#   helpful to use and save additive layer information
#
# Author Sergey Solohin (Neill) 2017
#   e-mail to: s@neill3d.com
# Homepage: www.neill3d.com
#
# 
##################################################

from pyfbsdk import *


#
def ExtractLayerIntoANewTake(modelList):

    lSystem = FBSystem()
    srcTake = lSystem.CurrentTake
    
    newTake = srcTake.CopyTake('Additive Animation')
    newTake.SetCurrentLayer(0)
    newTake.ClearAllPropertiesOnCurrentLayer()
    
    lPlayer = FBPlayerControl()
    lPlayer.GotoStart()
    
    lTime = FBTime()
    lTime = lSystem.LocalTime
    lTime = lTime - FBTime(0,0,0,1)
    t = FBVector3d(0.0, 0.0, 0.0)
    r = FBVector3d(0.0, 0.0, 0.0)
    
    # put a zero key
    for model in modelList:
        
        # rotation part
        lAnimProp = model.Rotation
        lRotNode = lAnimProp.GetAnimationNode()
                
        if lRotNode:
            lRotNode.Nodes[0].KeyAdd( lTime, r[0] )
            lRotNode.Nodes[1].KeyAdd( lTime, r[1] )
            lRotNode.Nodes[2].KeyAdd( lTime, r[2] )
    
        # translation part
        lAnimProp = model.Translation
        lMoveNode = lAnimProp.GetAnimationNode()
    
        if lMoveNode:
            lMoveNode.Nodes[0].KeyAdd( lTime, t[0] )
            lMoveNode.Nodes[1].KeyAdd( lTime, t[1] )
            lMoveNode.Nodes[2].KeyAdd( lTime, t[2] )
        
    newTake.MergeLayers(FBAnimationLayerMergeOptions.kFBAnimLayerMerge_AllLayers_CompleteScene, True, FBMergeLayerMode.kFBMergeLayerModeAutomatic )

#
# MAIN

modelList = FBModelList()
FBGetSelectedModels(modelList)

if len(modelList) > 0:
    ExtractLayerIntoANewTake(modelList)
else:
    FBMessageBox('AdditiveLayer_IntoNewTake', 'Please select models to extract', 'Ok')