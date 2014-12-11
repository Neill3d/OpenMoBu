
####################################################
# additive calculation script
#
# Author Sergey Solohin (Neill), e-mail to: Neill.Solow@gmail.com
# Homepage: www.neill3d.com
# All rights reserved.
#
# Scene must have 3 takes
# where 1 - action
# 2 - pose
# result - action - pose (take 003)
# 
##################################################

from pyfbsdk import *
import math

def MatrixInverse(M):
    out = FBMatrix()
    out.Identity()

    for i in range(0,4):
            d = M[i*4+i]
            if (d <> 1.0):
                for j in range(0,4):
                    out[i*4+j] /= d
                    M[i*4+j] /= d
                    
            for j in range(0,4):
                if (j <> i):
                    if (M[j*4+i] <> 0.0):
                        mulBy = M[j*4+i]
                        for k in range(0,4):
                            M[j*4+k] -= mulBy * M[i*4+k]
                            out[j*4+k] -= mulBy * out[i*4+k]

    return out

def MatrixFastInverse(M):
    #f32     Determinant;
    #neM4    & Matrix = (*this);

    Matrix = FBMatrix()

    #
    # Calculate the determinant.
    #
    Determinant = ( M[0] * ( M[5] * M[10] - M[6] * M[9] ) -
                    M[1] * ( M[4] * M[10] - M[6] * M[8] ) +
                    M[2] * ( M[4] * M[9] - M[5] * M[8] ) )

    if( math.fabs( Determinant ) < 0.0001 ):
        Matrix.Identity()
        return Matrix

    Determinant = 1.0 / Determinant

    #
    # Find the inverse of the matrix.
    #
    Matrix[0] =  Determinant * ( M[5] * M[10] - M[6] * M[9] )
    Matrix[1] = -Determinant * ( M[1] * M[10] - M[2] * M[9] )
    Matrix[2] =  Determinant * ( M[1] * M[6] - M[2] * M[5] )
    Matrix[3] = 0.0

    Matrix[4] = -Determinant * ( M[4] * M[10] - M[6] * M[8] )
    Matrix[5] =  Determinant * ( M[0] * M[10] - M[2] * M[8] )
    Matrix[6] = -Determinant * ( M[0] * M[6] - M[2] * M[4] )
    Matrix[7] = 0.0

    Matrix[8] =  Determinant * ( M[4] * M[9] - M[5] * M[8] )
    Matrix[9] = -Determinant * ( M[0] * M[9] - M[1] * M[8] )
    Matrix[10] =  Determinant * ( M[0] * M[5] - M[1] * M[4] )
    Matrix[11] = 0.0

    Matrix[12] = -( M[12] * Matrix[0] + M[13] * Matrix[4] + M[14] * Matrix[8] )
    Matrix[13] = -( M[12] * Matrix[1] + M[13] * Matrix[5] + M[14] * Matrix[9] )
    Matrix[14] = -( M[12] * Matrix[2] + M[13] * Matrix[6] + M[14] * Matrix[10] )
    Matrix[15] = 1.0

    return Matrix

def MatrixMult3(L, R):
    Temp = FBMatrix()

    for i in range(0,4):
        Temp[i*4]   = L[0] * R[i*4] + L[4] * R[i*4+1] + L[8] * R[i*4+2] + L[12] * R[i*4+3]
        Temp[i*4+1] = L[1] * R[i*4] + L[5] * R[i*4+1] + L[9] * R[i*4+2] + L[13] * R[i*4+3]
        Temp[i*4+2] = L[2] * R[i*4] + L[6] * R[i*4+1] + L[10] * R[i*4+2] + L[14] * R[i*4+3]
        Temp[i*4+3] = L[3] * R[i*4] + L[7] * R[i*4+1] + L[11] * R[i*4+2] + L[15] * R[i*4+3]

    return Temp

def MatrixMult2(a, b):
    dest = FBMatrix()
    dest.Identity()

    dest[0] = a[0] * b[0] + a[1] * b[4] + a[2] * b[8]
    dest[1] = a[0] * b[1] + a[1] * b[5] + a[2] * b[9]
    dest[2] = a[0] * b[2] + a[1] * b[6] + a[2] * b[10]
    
    dest[4] = a[4] * b[0] + a[5] * b[4] + a[6] * b[8]
    dest[5] = a[4] * b[1] + a[5] * b[5] + a[6] * b[9]
    dest[6] = a[4] * b[2] + a[5] * b[6] + a[6] * b[10]

    dest[8] = a[8] * b[0] + a[9] * b[4] + a[10] * b[8]
    dest[9] = a[8] * b[1] + a[9] * b[5] + a[10] * b[9]
    dest[10] = a[8] * b[2] + a[9] * b[6] + a[10] * b[10]

    dest[12] = a[12] * b[0] + a[13] * b[4] + a[14] * b[8] + b[12]
    dest[13] = a[12] * b[1] + a[13] * b[5] + a[14] * b[9] + b[13]
    dest[14] = a[12] * b[2] + a[13] * b[6] + a[14] * b[10] + b[14]

    return dest

def MatrixMult(Ma, Mb):
    res = FBMatrix()

    for i in range(0,4):
        for j in range(0,4):
            sum=0.0
            for k in range(0,4):
                sum += Ma[i*4+k] * Mb[k*4+j]

            res[i*4+j] = sum
    return res

def MatrixCheck(Ma, Mb):
    for i in range(0,16):
        if Ma[i] <> Mb[i]:
            print "> error"
            print i
            print Ma[i]
            print Mb[i]
            


def MatrixToEulerAngles( M ):

    heading = 0.0
    attitude = 0.0
    bank = 0.0

    if (M[4] > 0.998):
        heading = math.atan2(M[2], M[10] )
        attitude = math.pi /2
        bank = 0
    if (M[4] < -0.998):
        heading = math.atan2(M[2], M[10] )
        attitude = -math.pi /2
        bank = 0

    heading = math.atan2(-M[8], M[0])
    bank = math.atan2(-M[6], M[5])
    attitude = math.asin(M[4])

    heading = heading * 180 / math.pi
    attitude = attitude * 180 / math.pi
    bank = bank * 180 / math.pi

    res = FBVector3d( -1*bank, -1 * heading, -attitude )
    return res

def FBMatrixFromAnimationNode( pModel, pTime ):
    
    lResult = FBMatrix()
    if pModel == 0: return lResult
    
    lTranslationNode = pModel.Translation.GetAnimationNode()
    lRotationNode = pModel.Rotation.GetAnimationNode()
    lScaleNode = pModel.Scaling.GetAnimationNode()
 
    if not lRotationNode or not lScaleNode: return lResult
 
    lRotationV = FBVector3d(
        lRotationNode.Nodes[0].FCurve.Evaluate(pTime) * 0.017453292519943295769236907684886,
        lRotationNode.Nodes[1].FCurve.Evaluate(pTime) * 0.017453292519943295769236907684886,
        lRotationNode.Nodes[2].FCurve.Evaluate(pTime) * 0.017453292519943295769236907684886)
 
    lScaleV = FBVector3d(
        lScaleNode.Nodes[0].FCurve.Evaluate(pTime),
        lScaleNode.Nodes[1].FCurve.Evaluate(pTime),
        lScaleNode.Nodes[2].FCurve.Evaluate(pTime))
 
    sphi = math.sin(lRotationV[0])
    cphi = math.cos(lRotationV[0])
    stheta = math.sin(lRotationV[1])
    ctheta = math.cos(lRotationV[1])
    spsi = math.sin(lRotationV[2])
    cpsi = math.cos(lRotationV[2])
 
    lResult[0] = (cpsi*ctheta)*lScaleV[0]
    lResult[1] = (spsi*ctheta)*lScaleV[0]
    lResult[2] = (-stheta)*lScaleV[0]
 
    lResult[4] = (cpsi*stheta*sphi - spsi*cphi)*lScaleV[1]
    lResult[5] = (spsi*stheta*sphi + cpsi*cphi)*lScaleV[1]
    lResult[6] = (ctheta*sphi)*lScaleV[1]
 
    lResult[8] = (cpsi*stheta*cphi + spsi*sphi)*lScaleV[2]
    lResult[9] = (spsi*stheta*cphi - cpsi*sphi)*lScaleV[2]
    lResult[10] = (ctheta*cphi)*lScaleV[2]
 
    lResult[12] = lTranslationNode.Nodes[0].FCurve.Evaluate(pTime)
    lResult[13] = lTranslationNode.Nodes[1].FCurve.Evaluate(pTime)
    lResult[14] = lTranslationNode.Nodes[2].FCurve.Evaluate(pTime)
 
    return lResult

global poseNdx
poseTM = []
resultTM = []
names = []

def extractPoseTM( pModel, pTime, evalNames ):
    global poseTM

    lRotA = FBMatrix()
    lRotA = FBMatrixFromAnimationNode( pModel, pTime )

    poseTM.append(lRotA)
    if evalNames > 0:
        names.append( pModel.Name )
        global poseNdx
        poseNdx+=1

    for child in pModel.Children: extractPoseTM( child, pTime, evalNames )


def ProcessNode( pModel, pTime ):
    global poseTM, resultTM, poseNdx
    lRotA = FBMatrix()
    lRotA = FBMatrixFromAnimationNode( pModel, pTime )

    #print "> lRotA"
    #print lRotA

    lRotB = FBMatrix()
    #lRotB = poseTM[poseNdx]
    #poseNdx += 1
    lRotB = poseTM.pop(0)

    #print "> lRotB"
    #print lRotB

#    lRotB = MatrixFastInverse(lRotB)
    Minv = FBMatrix()
    Minv = MatrixFastInverse(lRotB)
    

    lM = FBMatrix()
    lM = MatrixMult(lRotA, Minv)

    #lCheck = FBMatrix()
    #lCheck = MatrixMult(lM, lRotB)
    #MatrixCheck(lCheck, lRotA)
    

    #print "> lM"
    #print lM

    resultTM.append(lM)
    names.append(pModel.Name)

    for child in pModel.Children: ProcessNode( child, pTime )


def SetResult( pModel, pTime ):

    lM = FBMatrix()
    lM = resultTM.pop(0)

    lMoveRes = FBVector3d()
    lMoveRes[0] = lM[12]
    lMoveRes[1] = lM[13]
    lMoveRes[2] = lM[14]
    
    lRotRes = FBVector3d()
    lRotRes = MatrixToEulerAngles( lM )

    # rotation part
    lAnimProp = pModel.Rotation
    lRotNode = lAnimProp.GetAnimationNode()
            
    if lRotNode:
        lRotNode.Nodes[0].KeyAdd( pTime, lRotRes[0] )
        lRotNode.Nodes[1].KeyAdd( pTime, lRotRes[1] )
        lRotNode.Nodes[2].KeyAdd( pTime, lRotRes[2] )

    # translation part
    lAnimProp = pModel.Translation
    lMoveNode = lAnimProp.GetAnimationNode()

    if lMoveNode:
        lMoveNode.Nodes[0].KeyAdd( pTime, lMoveRes[0] )
        lMoveNode.Nodes[1].KeyAdd( pTime, lMoveRes[1] )
        lMoveNode.Nodes[2].KeyAdd( pTime, lMoveRes[2] )

    for child in pModel.Children: SetResult( child, pTime )

def PrepareFile(startFrame, stopFrame):
    f=open('c:\\additive.dat', 'r+' )
    f.truncate(0)

# export time range
    print >> f, startFrame
    print >> f, stopFrame

# number of exported objects
    global poseNdx
    print >> f, poseNdx

# object names
    for name in names: print >> f, name

    f.close()

def ExportToFile( frame ):
    f=open('c:\\additive.dat', 'a')

    for tm in resultTM:
        print >> f, tm[0],' ',tm[1],' ',tm[2],' ',tm[4],' ',tm[5],' ',tm[6],' ',tm[8],' ',tm[9],' ',tm[10],' ',tm[12],' ',tm[13],' ',tm[14]

    f.close()
    


lSystem = FBSystem()
lApp = FBApplication()

# set third take as current
lSystem.CurrentTake = lSystem.Scene.Takes[2]

# let character put in stance pose
lCharacter = lApp.CurrentCharacter

if lCharacter:
    lCharacter.GoToStancePose(False, True)

    # go trough each frame
    lPlayer = FBPlayerControl()

    #lTemp = lSystem.LocalTime
    #lPlayer.GotoEnd()
    #lEndTime = FBTime()
    #lEndTime = lSystem.LocalTime
    #lPlayer.GotoStart()

    #print lEndTime.GetFrame(True)
    #lTime = FBTime()
    #lTime = lSystem.LocalTime
    #print lTime.GetFrame(True)

    #lFrame = lTime.GetFrame(True)
    #lEndFrame = lEndTime.GetFrame(True)
    lFrame = lPlayer.LoopStart.GetFrame(True)
    lEndFrame = lPlayer.LoopStop.GetFrame(True)

    #print lFrame
    #print lEndFrame

    #lEndFrame = 0
    #lFrame = 100

    lSystem.CurrentTake = lSystem.Scene.Takes[1]

    startIndex = FBBodyNodeId.kFBHipsNodeId
    rootModel = lCharacter.GetModel(FBBodyNodeId(startIndex))

        #lFrame = 8
        #lEndFrame = 8

    lTime = FBTime()
    lTime.SetTime( 0,0,0, lFrame )
 
    global poseNdx
    poseNdx = 0
    extractPoseTM( rootModel, lTime, 1 )

        #PrepareFile( lFrame, lEndFrame )

    while lFrame <= lEndFrame:
        lTime.SetTime( 0,0,0, lFrame )

        lSystem.CurrentTake = lSystem.Scene.Takes[1]
        extractPoseTM( rootModel, lTime, 0 )
                
        # compute local TM for this frame        
        lSystem.CurrentTake = lSystem.Scene.Takes[0]
                
        ProcessNode( rootModel, lTime )

        # export result to file
        #ExportToFile( lFrame )
                              
        # write result to third take
        lSystem.CurrentTake = lSystem.Scene.Takes[2]
        SetResult( rootModel, lTime )

        lFrame = lFrame + 1
