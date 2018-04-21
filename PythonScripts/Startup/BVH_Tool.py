
#########################################################################################
##
## BVH Import\Export Tool for Autodesk MotionBuilder
##
##  OpenMoBu project on GitHub - https://github.com/Neill3d/OpenMoBu
##
## Author Sergei Solokhin (Neill3d) 2014-2018
##  e-mail to: s@neill3d.com
##        www.neill3d.com
##
##  Lisenced under the "New" BSD License - https://github.com/Neill3d/OpenMoBu/blob/master/LICENSE
##
#########################################################################################

from pyfbsdk import *
from pyfbsdk_additions import *
import os
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

# BVHWriter
class BVHWriter:
    
    def __init__(self, filename, root):
        self.filename = filename
        self.root = root
        self.motionChannels = []
        
    def write(self):
        
        self.fhandle = file(self.filename, 'w')
        
        self.writeHeirarchy()
        self.writeMotion()
        
        self.fhandle.close()
        
    def writeHeirarchy(self):
        self.fhandle.write("HIERARCHY\n")
        
        self.writeNode(self.root, 0, False)
    
    def checkChannels(self, animNode):
        
        if animNode and len(animNode.Nodes) == 3:
            
            xAnimNode = animNode.Nodes[0]
            yAnimNode = animNode.Nodes[1]
            zAnimNode = animNode.Nodes[2]
            
            return (xAnimNode.KeyCount > 0 or yAnimNode.KeyCount > 0 or zAnimNode.KeyCount > 0)
        
        return False
        
    def writeTabLevel(self, level):
        
        for i in range(level):
            self.fhandle.write('\t')
    
    def writeNode(self, node, level, onlyOneChildren):
        
        IsEndSite = False
        
        # write name
        
        self.writeTabLevel(level)
        if level == 0:
            self.fhandle.write("ROOT " + node.Name + '\n')
        elif onlyOneChildren and (len(node.Children) == 0):
            self.fhandle.write("End Site\n")
            IsEndSite = True
        else:
            self.fhandle.write("JOINT " + node.Name + '\n')
        
        self.writeTabLevel(level)
        self.fhandle.write("{" + '\n')
        
        level += 1
        
        # write offset
        offset = FBVector3d()
        node.GetVector(offset, FBModelTransformationType.kModelTranslation, False)
        
        self.writeTabLevel(level)
        self.fhandle.write("OFFSET %.5f %.5f %.5f\n" % (offset[0], offset[1], offset[2]) )
        
        # write channels
            
        if IsEndSite == False:
        
            numberOfChannels = 0
            channels = []
        
            if self.checkChannels(node.Translation.GetAnimationNode()):
                numberOfChannels += 3
                channels += ["Xposition", "Yposition", "Zposition"]
    
                animNode = node.Translation.GetAnimationNode()
                self.motionChannels += [animNode.Nodes[0], animNode.Nodes[1], animNode.Nodes[2]]
            
            if self.checkChannels(node.Rotation.GetAnimationNode()):
                numberOfChannels += 3
                channels += ["Zrotation", "Yrotation", "Xrotation"]
            
                animNode = node.Rotation.GetAnimationNode()
                self.motionChannels += [animNode.Nodes[2], animNode.Nodes[1], animNode.Nodes[0]]
            
            self.writeTabLevel(level)
            self.fhandle.write("CHANNELS " + str(numberOfChannels))
            for ch in channels:
                self.fhandle.write( " " + ch )
            
            self.fhandle.write( '\n' )
        
        # write all children
        onlyOneChildren = len(node.Children) == 1
        for obj in node.Children:
            self.writeNode(obj, level, onlyOneChildren)
        
        level -= 1
        if level < 0: level = 0
        
        self.writeTabLevel(level)
        self.fhandle.write("}\n")
    
    def writeMotion(self):
        self.fhandle.write("MOTION\n")
        
        lControl = FBPlayerControl()
        startFrame = lControl.ZoomWindowStart.GetFrame()
        stopFrame = lControl.ZoomWindowStop.GetFrame()
        
        numberOfFrames = stopFrame - startFrame + 1
        frameTime = FBTime(0,0,0, 1).GetSecondDouble()
        
        self.fhandle.write("Frames: " + str(numberOfFrames) + '\n')
        self.fhandle.write("Frame Time: %.7f\n" % (frameTime) )
        
        for iFrame in range(numberOfFrames):
            self.writeMotionFrame(iFrame)
            self.fhandle.write('\n')
    
    def writeMotionFrame(self, iFrame):
        
        currTime = FBTime(0,0,0, iFrame)
        
        for animNode in self.motionChannels:
            if animNode.FCurve:
                value = animNode.FCurve.Evaluate(currTime)
                self.fhandle.write("%.4f " % (value) )
        
    
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

            if node.RotationOrder != FBModelRotationOrder.kFBEulerXYZ:
            
                eul[0]=eul[0]*DEG_TO_RAD
                eul[1]=eul[1]*DEG_TO_RAD
                eul[2]=eul[2]*DEG_TO_RAD
    
                qx = AxisAngleToQuaternion(1.0, 0.0, 0.0, eul[0])
                qy = AxisAngleToQuaternion(0.0, 1.0, 0.0, eul[1])
                qz = AxisAngleToQuaternion(0.0, 0.0, 1.0, eul[2])
    
                qres = FBVector4d(0.0, 0.0, 0.0, 1.0)
                if node.RotationOrder == FBModelRotationOrder.kFBEulerZXY:
                    FBQMult(qres, qy, qres)
                    FBQMult(qres, qx, qres)
                    FBQMult(qres, qz, qres)
                elif node.RotationOrder == FBModelRotationOrder.kFBEulerZYX:
                    FBQMult(qres, qx, qres)
                    FBQMult(qres, qy, qres)
                    FBQMult(qres, qz, qres)
                else:
                    print "unsupported mode"
    
                rotEuler = FBVector3d(0.0, 0.0, 0.0)
                FBQuaternionToRotation(rotEuler, qres, FBRotationOrder.kFBXYZ)
               
                rot = [rotEuler[0], rotEuler[1], rotEuler[2]]
            #

            animNode.KeyAdd(lTime, rot)
        
        values = values[nc:]
        for c in node.children:
            values = self.applyMotion(c, values)
            
        model.PropertyList.Find('RotationPivot').Data = FBVector3d(0.0, 0.0, 0.0)
        model.PropertyList.Find('RotationOffset').Data = FBVector3d(0.0, 0.0, 0.0)
            
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
            node.RotationOrder = order
            print order
            #j.RotationOrder = order
            # set local node offset
            
            #j.SetVector(FBVector3d(node.offset), FBModelTransformationType.kModelTranslation, True)
            
            j.Translation.SetAnimated(True)
            j.Rotation.SetAnimated(True)
            j.Translation = FBVector3d(node.offset)
            
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

####################################### UI ############

def OnButtonImport(control, event):

    lDialog = FBFilePopup()
    lDialog.Caption = "Please choose a file to import"
    lDialog.Filter = "*.bvh"
    lDialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    
    if lDialog.Execute():

        importer = BVHReader(lDialog.FullFilename)
        importer.read()
    
def OnButtonExport(control, event):

    lDialog = FBFilePopup()
    lDialog.Caption = "Please choose a file to export"
    lDialog.Filter = "*.bvh"
    lDialog.Style = FBFilePopupStyle.kFBFilePopupSave
    
    l = FBModelList()
    FBGetSelectedModels(l)
    
    if len(l) > 0 and lDialog.Execute():
        
        root = l[0]
        
        exporter = BVHWriter(lDialog.FullFilename, root)
        exporter.write()

def PopulateTool(t):
    
    # Create Main region frame:
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    main = FBVBoxLayout()
    t.AddRegion("main","main", x, y, w, h)
    t.SetControl("main",main)

    #
    b = FBButton()
    b.Caption = "Import BVH..."
    b.OnClick.Add(OnButtonImport)
    main.Add(b, 35)

    #
    b = FBButton()
    b.Caption = "Export BVH..."
    b.OnClick.Add(OnButtonExport)
    main.Add(b, 35)


def CreateTool():
    t = None
    try:
        t = FBCreateUniqueTool("BVH Tool")
    except NameError:
        t = CreateUniqueTool("BVH Tool")
        print "supporting MoBu 2010"
        
    if t:
        t.StartSizeX = 200
        t.StartSizeY = 120
        PopulateTool(t)
        ShowTool(t)
        
CreateTool()

        