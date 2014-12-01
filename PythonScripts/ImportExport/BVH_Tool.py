
#########################################################################################
##
## BVH Import\Export Tool for Autodesk MotionBuilder
##
##  MoBu project on GitHub - https://github.com/Neill3d/MoBu
##
## Author Sergey Solohin (Neill3d) 2014
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
##
##  Lisenced under the "New" BSD License - https://github.com/Neill3d/MoBu/blob/master/LICENSE
##
#########################################################################################

from pyfbsdk import *
import sys
import bvh
import math

DEG_TO_RAD=            math.pi/180.0
RAD_TO_DEG=            180.0/math.pi


def AxisAngleToQuaternion( ax, ay, az, angle ):

    qx = ax * math.sin(0.5 * angle)
    qy = ay * math.sin(0.5 * angle)
    qz = az * math.sin(0.5 * angle)
    qw = math.cos(0.5 * angle)

    return FBVector4d(qx, qy, qz, qw)

# BVHReader

class BVHReader(bvh.BVHReader):
    
    def __init__(self, filename):
        bvh.BVHReader.__init__(self, filename)
        
    def onHierarchy(self, root):
        self.createSkeleton(root)
        self.root = root
        
    def onMotion(self, frames, dt):
        self.frames = frames
        self.dt = dt
        self.currentframe = 0
        
    def onFrame(self, values):
        self.applyMotion(self.root, values)
        self.currentframe += 1
        
    def applyMotion(self, node, values):
        """Apply a motion sample to the skeleton.

        node is the current joint and values the joint angles for the
        entire skeleton.
        The method returns the remaining joint angles.
        """
        
        t = self.currentframe*self.dt
        lTime = FBTime(0)
        lTime.SetSecondDouble(t)
        
        nc = len(node.channels)
        vals = values[:nc]
        pos = [0.0, 0.0, 0.0]
        rot = [0.0, 0.0, 0.0]
        pos_flag = False
        
        model = node.joint
        
        for ch,v in zip(node.channels, vals):
            if ch=="Xrotation":
                rot[0] = v
            elif ch=="Yrotation":
                rot[1] = v
            elif ch=="Zrotation":
                rot[2] = v
            elif ch=="Xposition":
                pos[0] = v
                pos_flag = True
            elif ch=="Yposition":
                pos[1] = v
                pos_flag = True
            elif ch=="Zposition":
                pos[2] = v
                pos_flag = True
        
        if pos_flag:
            # add a position keyframe
            animNode = model.Translation.GetAnimationNode()
            if animNode:
                animNode.KeyAdd(lTime, pos)
                
        # add a rotation keyframe
        animNode = model.Rotation.GetAnimationNode()
        if animNode:
            eul = [rot[0], rot[1], rot[2]]
            eul[0]=eul[0]*DEG_TO_RAD
            eul[1]=eul[1]*DEG_TO_RAD
            eul[2]=eul[2]*DEG_TO_RAD

            qx = AxisAngleToQuaternion(1.0, 0.0, 0.0, eul[0])
            qy = AxisAngleToQuaternion(0.0, 1.0, 0.0, eul[1])
            qz = AxisAngleToQuaternion(0.0, 0.0, 1.0, eul[2])

            qres = FBVector4d(0.0, 0.0, 0.0, 1.0)
            FBQMult(qres, qy, qres)
            FBQMult(qres, qx, qres)
            FBQMult(qres, qz, qres)

            rotEuler = FBVector3d(0.0, 0.0, 0.0)
            FBQuaternionToRotation(rotEuler, qres, FBRotationOrder.kFBXYZ)
           
            rot = [rotEuler[0], rotEuler[1], rotEuler[2]]

            animNode.KeyAdd(lTime, rot)
        
        values = values[nc:]
        for c in node.children:
            values = self.applyMotion(c, values)
        return values
        
    def createSkeleton(self, node, parent=None):
        """Create the skeleton hierarchy.

        This method creates the skeleton recursively. Each invocation
        creates one joint.
        """
        
        order = self.rotationOrder(node.channels)
        # Create a new skeleton node
        j = FBModelSkeleton(node.name)
        if j:
            j.Parent = parent
            #node.RotationOrder = order
            #j.RotationOrder = order
            # set local node offset
            
            j.SetVector(FBVector3d(node.offset), FBModelTransformationType.kModelTranslation, True)
            
            j.Translation.SetAnimated(True)
            j.Rotation.SetAnimated(True)
            
            j.Show = True
            
        node.joint = j
        
        
        for c in node.children:
            self.createSkeleton(c, j)
            
    def rotationOrder(self, channels):
        """Determine rotation order FBModelRotationOrder from the channel names.
        """
        
        res = FBModelRotationOrder.kFBEulerXYZ

        resStr = ""
        for c in channels:
            if c[-8:]=="rotation":
                resStr += c[0]

        # Complete the order string if it doesn't already contain
        # all three axes
        m = { "":FBModelRotationOrder.kFBEulerXYZ,
              "X":FBModelRotationOrder.kFBEulerXYZ, "Y":FBModelRotationOrder.kFBEulerYXZ, "Z":FBModelRotationOrder.kFBEulerZXY,
              "XY":FBModelRotationOrder.kFBEulerXYZ, "XZ":FBModelRotationOrder.kFBEulerXZY,
              "YX":FBModelRotationOrder.kFBEulerYXZ, "YZ":FBModelRotationOrder.kFBEulerYZX,
              "ZX":FBModelRotationOrder.kFBEulerZXY, "ZY":FBModelRotationOrder.kFBEulerZYX }
        if resStr in m:
            res = m[resStr]
        return res

#######################################

importer = BVHReader("C:\\Work\\BVH\\test_01_01.bvh")
importer.read()


        