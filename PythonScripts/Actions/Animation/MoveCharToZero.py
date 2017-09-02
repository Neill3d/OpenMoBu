
############################################################
#
# Script for moving character to the world zero
#
# Author Sergey Solohin (Neill3d)
# homepage: http://neill3d.com
# e-mail to: s@neill3d.com
# 2012
############################################################

from pyfbsdk import *

currChar = FBApplication().CurrentCharacter
if currChar:
    hipsModel = currChar.GetEffectorModel(FBEffectorId.kFBHipsEffectorId)
    refModel = currChar.GetCtrlRigModel(FBBodyNodeId.kFBReferenceNodeId)
    
    if hipsModel and refModel:
        v = FBVector3d()
        v2 = FBVector3d()
        hipsModel.GetVector(v)
        refModel.GetVector(v2)
        v[0] = v2[0] - v[0]
        v[1] = v2[1]
        v[2] = v2[2] - v[2]
        refModel.SetVector(v)
