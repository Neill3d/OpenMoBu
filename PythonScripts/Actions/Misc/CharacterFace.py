
from pyfbsdk import *

modelList = FBModelList()
#FBGetSelectedModels(modelList)

lSystem = FBSystem()

lCharFace = None
if len(lSystem.Scene.CharacterFaces) > 0:
    lCharFace = lSystem.Scene.CharacterFaces[0]
    
if lCharFace: # and len(modelList) > 0:
    print 'add new group'
    res = lCharFace.ShapeGroupAdd(None, None)
    print res