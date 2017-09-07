
from pyfbsdk import *

models = FBModelList()
FBGetSelectedModels(models)

lPlayer = FBPlayerControl()
lApp = FBApplication()

lTime = FBTime()
lStartFrame = lPlayer.LoopStart.GetFrame()
lStopFrame = lPlayer.LoopStop.GetFrame()

lProgress = FBProgress()
lProgress.ProgressBegin()
lProgress.Caption = 'Computing'
lProgress.Percent = 0

lApp.UpdateAllWidgets()
lApp.FlushEventQueue()

lTime.SetTime( 0,0,0, lStartFrame)  
lPlayer.Goto(lTime)
lTemp = []

for model in models:
    v = FBVector3d()
    model.GetVector(v)
    lTemp.append(v)

    prop = model.PropertyList.Find('accel')
    if prop is not None:
        prop.SetAnimated(True)

for lFrame in range(lStartFrame, lStopFrame+1):
    lTime.SetTime( 0,0,0, lFrame)    
    lPlayer.Goto(lTime)
    
    if True == lProgress.UserRequestCancell():
        break
    
    ##
    for i in range( len(models) ):
        model = models[i]
        oldV = lTemp[i]

        v = FBVector3d()
        model.GetVector(v)
        
        dist = FBLength( FBVector4d( oldV[0]-v[0], oldV[1]-v[1], oldV[2]-v[2], 0.0) )
        
        prop = model.PropertyList.Find('accel')
        if prop is not None:
            prop.GetAnimationNode().KeyAdd(dist)
    
        lTemp[i] = v
    #
    p = 100.0 * lFrame / (lStopFrame-lStartFrame)
    lProgress.Percent = int(p)
    
lProgress.ProgressDone()