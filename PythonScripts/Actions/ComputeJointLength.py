
# helper script for stretch IK
#   write joint length into a user property

from pyfbsdk import *

models = FBModelList()
FBGetSelectedModels(models)

for model in models:
    prop = model.PropertyList.Find('JointLength')
    if prop is None:
        prop = model.PropertyCreate('JointLength', FBPropertyType.kFBPT_double, 'Number',True, True, None)
        
    value = 0.0
    
    if model.Parent is not None:
        v = FBVector3d()
        v2 = FBVector3d()
        
        model.GetVector(v)
        model.Parent.GetVector(v2)
        
        value = FBLength( FBVector4d(v2[0]-v[0], v2[1]-v[1], v2[2]-v[2], 0.0) )
        
    prop.Data = value