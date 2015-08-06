
###############################
#
# Simple script for creating a circle
#
# Sergey Solohin (Neill3d) 2015
#  e-mail to: s@neill3d.com
#   www.neill3d.com
#
###############################

from pyfbsdk import *

curve = FBModelPath3D("circle")
curve.Show = True
curve.Visible = True

curve.PathKeyEndAdd( FBVector4d(-1.0, 0.0, 0.0, 1.0) )
curve.PathKeyEndAdd( FBVector4d(0.0, 0.0, 1.0, 1.0) )
curve.PathKeyEndAdd( FBVector4d(1.0, 0.0, 0.0, 1.0) )
curve.PathKeyEndAdd( FBVector4d(0.0, 0.0, -1.0, 1.0) )
curve.PathKeyEndAdd( FBVector4d(-1.0, 0.0, 0.0, 1.0) )


#PATCH: Remove the two first point, they are unnecessary
curve.PathKeyRemove(0)
curve.PathKeyRemove(0)

# tangents

curve.PathKeySetLeftTangent(1, FBVector4d(-0.5, 0.0, 1.0, 1.0))
curve.PathKeySetRightTangent(1, FBVector4d(0.5, 0.0, 1.0, 1.0))

curve.PathKeySetLeftTangent(2, FBVector4d(1.0, 0.0, 0.5, 1.0))
curve.PathKeySetRightTangent(2, FBVector4d(1.0, 0.0, -0.5, 1.0))

curve.PathKeySetLeftTangent(3, FBVector4d(0.5, 0.0, -1.0, 1.0))
curve.PathKeySetRightTangent(3, FBVector4d(-0.5, 0.0, -1.0, 1.0))

curve.PathKeySetRightTangent(0, FBVector4d(-1.0, 0.0, 0.5, 1.0))
curve.PathKeySetLeftTangent(curve.PathKeyGetCount()-1, FBVector4d(-1.0, 0.0, -0.5, 1.0))