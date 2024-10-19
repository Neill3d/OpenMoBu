
###############################
#
# Hard Plotting.py
#  Script helps to plot animation with a relation to deformable meshes
# For instance, in attachment constraint, when every frame scene evaluation is required
#  normal plot operation would skip such evaluations, 
#  while going frame by frame helps to preserve the deformed data and keep attachments on place
#
# Sergei Solokhin (Neill3d) 2018
#  e-mail to: s@neill3d.com
#   www.neill3d.com
#
###############################

from pyfbsdk import *
import time

l = FBModelList()
FBGetSelectedModels(l)

gApp = FBApplication()
gSystem = FBSystem()
player = FBPlayerControl()

startFrame = player.LoopStart
stopFrame = player.LoopStop

global curFrame
curFrame = startFrame
stepFrame = FBTime(0,0,0,1)
'''
for obj in l:
    obj.Translation.SetAnimated(False)
    obj.Translation.SetAnimated(True)
'''
v = FBVector3d()

def CaptureFrame(control, event):
    
    for obj in l:
        if obj.Translation.GetAnimationNode():
            obj.GetVector(v, FBModelTransformationType.kModelTranslation, False)
            for i in range(0,3):
                obj.Translation.GetAnimationNode().Nodes[i].KeyAdd(v[i])
        
        if obj.Rotation.GetAnimationNode():
            obj.GetVector(v, FBModelTransformationType.kModelRotation, False)
            for i in range(0,3):
                obj.Rotation.GetAnimationNode().Nodes[i].KeyAdd(v[i])
                
        if obj.Scaling.GetAnimationNode():
            obj.GetVector(v, FBModelTransformationType.kModelScaling, False)
            for i in range(0,3):
                obj.Scaling.GetAnimationNode().Nodes[i].KeyAdd(v[i])

#
# MAIN

while curFrame <= stopFrame:
    
    player.Goto(curFrame)
    # 1
    gSystem.Scene.Evaluate()
    gSystem.Scene.EvaluateDeformations()

    gApp.UpdateAllWidgets()
    gApp.FlushEventQueue()

    # 2 
    gSystem.Scene.Evaluate()
    gSystem.Scene.EvaluateDeformations()

    gApp.UpdateAllWidgets()
    gApp.FlushEventQueue()

    CaptureFrame(None, None)
    
    curFrame += stepFrame
    
