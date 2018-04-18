

#################################################################
#
# Script for cleaning unused and dublicated content in the scene
#
# Github repo - https://github.com/Neill3d/MoPlugs
# Licensed under BSD 3-clause
#  https://github.com/Neill3d/MoPlugs/blob/master/LICENSE
#
# Author: Sergei Solokhin (Neill3d) 2014-2018, e-mail to: s@neill3d.com
#  www.neill3d.com
#
# TODO:
#   - remove layered replaces only diffuse channel in the material
#   - remove dublicates in textures replaces only diffuse channel
#   - CompareMaterials function compares only ambient, diffuse colors and diffuse textures
#
# LOG
#   31.01.2018 - a small fix and cleanup
#   16.01.2014 - fix for dynamic textures and videos
#   11.05.2014 - check dublicate textures and video clips
#   29.03.2014 * skip deleting video clips that take part in story tracks
#   28.03.2014 - first version of a script
#
#
#################################################################


from pyfbsdk import *
from pyfbsdk_additions import *
from os.path import dirname, basename, splitext, exists, split
import shutil

from StringIO import StringIO
import sys
import math

gDEVELOPMENT = True

CLEANUP_MANAGER_VERSION = 0.62

gApp = FBApplication()
gSystem = FBSystem()

gConsole = FBMemo()
gStrings = FBStringList()

materialTextureTypes = [FBMaterialTextureType.kFBMaterialTextureAmbient, FBMaterialTextureType.kFBMaterialTextureAmbientFactor, 
    FBMaterialTextureType.kFBMaterialTextureBump, FBMaterialTextureType.kFBMaterialTextureDiffuse, FBMaterialTextureType.kFBMaterialTextureDiffuseFactor, 
    FBMaterialTextureType.kFBMaterialTextureEmissive, FBMaterialTextureType.kFBMaterialTextureEmissiveFactor, FBMaterialTextureType.kFBMaterialTextureNormalMap,
    FBMaterialTextureType.kFBMaterialTextureReflection, FBMaterialTextureType.kFBMaterialTextureReflectionFactor, FBMaterialTextureType.kFBMaterialTextureShiness, 
    FBMaterialTextureType.kFBMaterialTextureSpecular, FBMaterialTextureType.kFBMaterialTextureSpecularFactor, FBMaterialTextureType.kFBMaterialTextureTransparent, 
    FBMaterialTextureType.kFBMaterialTextureTransparentFactor]

#/////////////////////////////////////////////////////////////////////////////////////////////////////////
#// COMMON 
        
def cclear():
    global gStrings
    global gConsole
   
    gStrings.Clear()
    gStrings.Add( ">>>" )
    gConsole.SetStrings(gStrings)
    
def cprint(text):
    global gStrings
    global gConsole
    
    gStrings.Add(text)
    gConsole.SetStrings(gStrings)
    gConsole.Refresh()
    

#/////////////////////////////////////////////////////////////////////////////////////////////////////////
#// 

def TextureGetVideoPath(texture):
    
    if isinstance(texture, FBLayeredTexture):
        if len(texture.Layers) > 0:
            return TextureGetVideoPath(texture.Layers[0])
        
    elif isinstance(texture, FBTexture):
        video = texture.Video
        if video != None:
            if isinstance(video, FBVideoMemory):
                return video.Name
            elif isinstance(video, FBVideoClip):
                fname = video.Filename
                return fname
    return ""

# return True when vectors are the same
def CompareVectors(v1, v2):
    
    eps = 0.001
    
    for i in range(3):
        if ( abs(v1[i]-v2[i]) > eps ):
            return False
        
    return True

# return true when textures are the same
def CompareTextures(texture1, texture2):
    
    if texture1 is None or texture2 is None:
        return False
    
    path1 = TextureGetVideoPath(texture1)
    path2 = TextureGetVideoPath(texture2)
    
    if path1 == None: 
        print "FAILED to find path for texture1"
        return False
        
    if path2 == None: 
        print "FAILED to find path for texture2"
        return False
    
    if path1 != path2: return False
    
    v1 = texture1.Translation
    v2 = texture2.Translation
    if False == CompareVectors(v1, v2):
        return False
    
    v1 = texture1.Rotation
    v2 = texture2.Rotation
    if False == CompareVectors(v1, v2):
        return False
    
    v1 = texture1.Scaling
    v2 = texture2.Scaling
    if False == CompareVectors(v1, v2):
        return False
    
    return True

# return True when clips are equal
def CompareVideoClips(iVideo, jVideo):
    
    if (iVideo.Filename != jVideo.Filename):
        return False
    
    # check video sequence options
    if (iVideo.FreeRunning != jVideo.FreeRunning):
        return False
    elif (iVideo.LastFrame != jVideo.LastFrame):
        return False
    elif (iVideo.PlaySpeed != jVideo.PlaySpeed):
        return False
    elif (iVideo.StartFrame != jVideo.StartFrame):
        return False
    elif (iVideo.StopFrame != jVideo.StopFrame):
        return False
    elif (iVideo.TimeOffset != jVideo.TimeOffset):
        return False
    
    return True

def CompareColors(color1, color2):
    if color1[0] != color2[0] or color1[1] != color2[1] or color1[2] != color2[2]:
        return False
        
    return True

def CompareMaterials(mat1, mat2):
    
    #print "compare " + mat1.Name + " and " + mat2.Name
    
    if False == CompareColors(mat1.Ambient, mat2.Ambient): 
        #print "ambient failed"
        return False
    if False == CompareColors(mat1.Diffuse, mat2.Diffuse): 
        #print "diffuse failed"
        return False
    if False == CompareTextures(mat1.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuse), mat2.GetTexture(FBMaterialTextureType.kFBMaterialTextureDiffuse)):
        #print "diffuse textures failed"
        return False
    
    #print "THE SAME"
    return True
    
# replace all connections with src material to the dst material
def ReplaceMaterial( srcmat, dstmat ):
    
    result = False
    count = srcmat.GetDstCount()
    
    dstList = []
    for i in range(0, count):
        dstList.append(srcmat.GetDst(i))
    
    for dst in dstList:
        
        if isinstance(dst, FBModel):
            
            temp = []
            for mat in dst.Materials:
                temp.append(mat)
                
                if mat.Name == dstmat.Name:
                    # to avoid material dublication, lets keep several copies of material
                    result = False
                    return result

                
            dst.Materials.removeAll()
            
            for mat in temp:
                if mat.Name == srcmat.Name:
                    print "REPLACE " + srcmat.Name + " with " + dstmat.Name 
                    dst.Materials.append(dstmat)                   
                    result = True
                else:
                    #print "adding - " + mat.Name
                    dst.Materials.append(mat)  
                    
            del (temp)                  
                    
    return result

def ReplaceTexture( textureToReplace, textureToPut ):
    count = textureToReplace.GetDstCount()
    dstArray = []
    for j in range(count):
        dstArray.append( textureToReplace.GetDst(j) )
        
    for dst in dstArray:
     
        if dst.Name == "DiffuseColor" and isinstance(dst, FBProperty):
            owner = dst.GetOwner()
       
            if owner and isinstance(owner, FBMaterial):
                owner.SetTexture(textureToPut, FBMaterialTextureType.kFBMaterialTextureDiffuse)
                
            #
        #

def DoCleanUp():

    cclear()
    cprint( "== CLEANING OPERATION == " )
    cprint( "" )

    global popupControls
    CleanUpMaterials =           (popupControls[0].State == 1)
    CleanUpMaterialsDublicates = (popupControls[1].State == 1)
    CleanUpMaterialsUnUsed =    (popupControls[2].State == 1)
    CleanUpMaterialsDelete =    (popupControls[3].State == 1)
    CleanUpTextures =           (popupControls[4].State == 1)
    CleanUpTexturesDublicates = (popupControls[5].State == 1)
    CleanUpTexturesReplaceLayered = (popupControls[6].State == 1)
    CleanUpTexturesUnUsed =         (popupControls[7].State == 1)
    CleanUpTexturesDelete =         (popupControls[8].State == 1)
    CleanUpVideoClips =             (popupControls[9].State == 1)
    CleanUpVideoClipsDublicates =   (popupControls[10].State == 1)
    CleanUpVideoClipsUnUsed =       (popupControls[11].State == 1)
    CleanUpVideoClipsDelete =       (popupControls[12].State == 1)
    CleanUpShaders =            (popupControls[13].State == 1)
    CleanUpShadersDelete =      (popupControls[14].State == 1)
    numDeletedClips = 0

    #
    lScene = gSystem.Scene

    #
    # process textures
    #
    modelList = FBModelList()
    FBGetSelectedModels(modelList)
    videoList = []
    videoFlags = []
    if CleanUpVideoClips:
        for lvideo in lScene.VideoClips:
            if isinstance(lvideo, FBVideoClip) and not isinstance(lvideo, FBVideoMemory):
                videoList.append(lvideo)
                videoFlags.append(False)
            
    textureList = []
    textureFlags = []
    if CleanUpTextures:
        if len(modelList) > 0:
            for lModel in modelList:
                for lMat in lModel.Materials: 
                    ProcessMaterial(textureList, textureFlags, lMat)
        else:
            for texture in lScene.Textures:
                textureList.append(texture)
                textureFlags.append(False)
          
    materialList = []
    materialFlags = []
    if CleanUpMaterials:
        for lMaterial in lScene.Materials:
            if lMaterial.Name == "DefaultMaterial":
                continue
                
            materialList.append(lMaterial)     
            materialFlags.append(False)
        # next
    
    shaderList = []
    if CleanUpShaders:
        for lShader in lScene.Shaders:

            if lShader.Name != "Default Shader":
                shaderList.append(lShader)

    # process materials (cleanup dublicated)
    numDeletedMaterials = 0
    
    # process materials (cleanup unused)
    #CleanUpMaterialsUnUsed = True
    if CleanUpMaterialsUnUsed:
        for i, lMaterial in enumerate(materialList):
            
            if True == materialFlags[i]:
                continue
                
            count = lMaterial.GetDstCount()
            
            IsConnected = False
            for j in range(count):
                dst = lMaterial.GetDst(j)
                
                if isinstance(dst, FBModel):
                    IsConnected = True
                    break
                    
            if False == IsConnected:
                cprint( "Delete unused - " + lMaterial.Name )                
                materialFlags[i] = True

    
    #CleanUpMaterialsDublicates = True
    if CleanUpMaterialsDublicates:
        progress = FBProgress()
        
        progress.Caption = 'CleanUp'
        progress.Text = 'Search for material dublicates'
        progress.ProgressBegin()
        
        matCount = len(materialList)
        for i, iMaterial in enumerate(materialList):
            
            # skip deleted material
            if True == materialFlags[i]:
                continue
            
            # check material in dict (with flag for deleting and skip in comparing operation)
            for j, jMaterial in enumerate(materialList):
                if i == j or True == materialFlags[j]:
                    continue
                    
                # delete j material if it's equeal to i'th material
                if True == CompareMaterials(iMaterial, jMaterial):
                    
                    # delete only if this material is unique!
                    if ReplaceMaterial(jMaterial, iMaterial):
                        materialFlags[j] = True
                progress.Percent = int(1.0 * i / matCount)
            
        progress.ProgressDone()
    
    # delete materials after comparing
    materialsToDelete = []
    for flag, lMaterial in zip(materialFlags, materialList):
        if True == flag:
            materialsToDelete.append(lMaterial)
    
    numDeletedMaterials = len(materialsToDelete)
    if True == CleanUpMaterialsDelete:
        [x.FBDelete() for x in materialsToDelete]               

    #
    # process textures (cleanup unused)
    #
    #CleanUpTexturesDelete = True
    numDeletedTextures = 0 
    #print textureFlags                   
    for i, lTexture in enumerate(textureList):
        
        if True == textureFlags[i]:
            continue
       
        if True == CleanUpTexturesReplaceLayered and isinstance(lTexture, FBLayeredTexture):
            #print "layered"
            if 0 == len(lTexture.Layers):
                
                textureFlags[i] = True
                #print "DELETE - " + str(i) + " - " + lTexture.Name
                #if CleanUpTexturesDelete:
                #    lTexture.FBDelete()
                numDeletedTextures += 1
                
            elif 1 == len(lTexture.Layers):
                #cleanup, no need to continue with layered lTexture
                ReplaceTexture( lTexture, lTexture.Layers[0] )
                lTexture.Layers.removeAll()
                #print "DELETE - " + str(i) + " - " + lTexture.Name
                
                textureFlags[i] = True
            else:
                print "MORE THEN ONE LAYER"
    
    #CleanUpTexturesUnUsed = True  
    #print textureFlags  
    if CleanUpTexturesUnUsed:
        for i, lTexture in enumerate(textureList):
                        
            if True == textureFlags[i]:
                continue

            count = lTexture.GetDstCount()
            
            IsConnected = False
            for j in range(count):
                dst = lTexture.GetDst(j)
                  
                if isinstance(dst, FBProperty):
                    IsConnected = True
                    break
            
            # check for dynamic textures (composite master, dynamic mask, etc.)
            count = lTexture.GetSrcCount()
            for j in range(count):
                src = lTexture.GetSrc(j)
                if isinstance(src, FBVideoMemory):
                    IsConnected = True
                    break
            
            if False == IsConnected:
                
                if isinstance(lTexture, FBLayeredTexture):
                    lTexture.Layers.removeAll()
                #print "DELETE - " + lTexture.Name
                textureFlags[i] = True
        #
    
    # check dublicates
    if CleanUpTexturesDublicates:
        
        for i, iTexture in enumerate(textureList):
            iTexture = textureList[i]
            
            for j, jTexture in enumerate(textureList):
                
                if (i!=j) and (False == textureFlags[i]) and (False == textureFlags[j]):
                    # compate and replace j with i
                    if CompareTextures(iTexture, jTexture):
                        
                        ReplaceTexture(jTexture, iTexture)
                        textureFlags[j] = True
    
    texturesToDelete = []
    for flag, lTexture in zip(textureFlags, textureList):
        if True == flag:
            texturesToDelete.append(lTexture)

    numDeletedTextures = len(texturesToDelete)
    if True == CleanUpTexturesDelete:
        [x.FBDelete() for x in texturesToDelete]

    
    #
    # try to find unused video clips
    #
    
    # CleanUpVideoClipsDublicates = True
    if CleanUpVideoClipsDublicates:

        for i, iVideo in enumerate(videoList):

            for j, jVideo in enumerate(videoList):

                if (i!=j) and (False == videoFlags[i]) and (False == videoFlags[j]):
                    
                    # compare and replace j with i
                    if (iVideo.Filename != "") and True == CompareVideoClips(iVideo, jVideo):
                        
                        dstCount = jVideo.GetDstCount()
                        
                        for k in range(dstCount):
                            dst = jVideo.GetDst(k)
                            
                            if isinstance(dst, FBScene):
                                continue
                            elif isinstance(dst, FBTexture):
                                # replace here
                                dst.Video = iVideo
                                videoFlags[j] = True
                                
                            elif isinstance(dst, FBComponent):
                                dstCount2 = dst.GetDstCount()
                                
                                for l in range(dstCount2):
                                    dst2 = dst.GetDst(l)
                                    
                                    if isinstance(dst2, FBScene):
                                        continue
                                    elif isinstance(dst2, FBTexture):
                                        # replace here
                                        dst2.Video = iVideo
                                        videoFlags[j] = True
    
    # CleanUpVideoClipsUnUsed = True
    if CleanUpVideoClipsUnUsed:
        
        for idx, lVideo in enumerate(videoList):
            
            count = lVideo.GetDstCount()
            
            IsConnected = False
            for i in range(count):
                dst = lVideo.GetDst(i)
                
                if isinstance(dst, FBScene):
                    continue
                
                if isinstance(dst, FBComponent):
                    
                    count2 = dst.GetDstCount()
                    for j in range(count2):
                        dst2 = dst.GetDst(j)
                        
                        if isinstance(dst2, FBScene):
                            continue
                        if isinstance(dst2, FBTexture) or isinstance(dst2, FBStoryTrack) or isinstance(dst2, FBVideoClip):
                            IsConnected = True
                            break
                
                if isinstance(dst, FBTexture) or isinstance(dst, FBStoryTrack):
                    IsConnected = True
                    break
                        
            if IsConnected == False:
                videoFlags[idx] = True
            #
        
    clipsToDelete = []    
    for flag, lVideo in zip(videoFlags, videoList):
        
        if True == flag:
            clipsToDelete.append(lVideo)
        
    numDeletedClips = len(clipsToDelete)
    if True == CleanUpVideoClipsDelete:
        [x.FBDelete() for x in clipsToDelete]

    #
        
    #
    ## SHADERS
    #
    shadersToDelete = []
    
    for lShader in shaderList:
        
        IsConnected = False
        count = lShader.GetDstCount()
        for i in range(count):
            dst = lShader.GetDst(i)
            
            if isinstance(dst, FBModel):
                IsConnected = True
                break
        
        if False == IsConnected:
            shadersToDelete.append(lShader)

    #
                
    numDeletedShaders = len(shadersToDelete)
    if True == CleanUpShadersDelete:
        [x.FBDelete() for x in shadersToDelete]
        
    # print info
    msg = "Deleted materials - " + str(numDeletedMaterials) + ' of ' + str(len(materialList))
    msg = msg + "; \ndeleted textures - " + str(numDeletedTextures) + ' of ' + str(len(textureList))
    msg = msg + "; \ndeleted media clips - " + str(numDeletedClips) + ' of ' + str(len(videoList))
    msg = msg + "; \ndeleted shaders - " + str(numDeletedShaders) + ' of ' + str(len(shaderList))
    
    cprint(msg)
    
    FBMessageBox("Texture Manager", msg, "Ok" )

def ProcessTexture(somelist, flaglist, texture):
    if texture: 
        somelist.append(texture)
        flaglist.append(False)
        if isinstance(texture, FBLayeredTexture):
    
            for ltexture in texture.Layers:
                somelist.append(ltexture)
                flaglist.append(False)


def ProcessMaterial(textureList, flaglist, mat):
    
    for textureType in materialTextureTypes:
        ProcessTexture( textureList, flaglist, mat.GetTexture(textureType) )
    
    ##
         
        
def ButtonAboutClickEvent(control, event):
    FBMessageBox("About", "Version: " + str(CLEANUP_MANAGER_VERSION) + "\n\n Author Sergei <Neill3d> Solohin (c) 2014-2018\n e-mail to:s@neill3d.com\n www.neill3d.com\n\nThanks for Cam Christiansen", "Ok")
    

def ButtonDoItCallback(control, event):    
    DoCleanUp()

def ButtonDblClickEvent(control, event):
    print "DblClick"

def PopulateVBox(names, justify, box):
    global popupControls
    
    idx = 0
    for name in names:
        b = FBButton()
        b.Caption = name
        b.Style = FBButtonStyle.kFBCheckbox
        b.Look = FBButtonLook.kFBLookColorChange
        b.Hint = "Click to switch the button state"
        #b.Justify = FBTextJustify( justify[idx] )
        
        if justify[idx] == 0:
            b.SetStateColor( FBButtonState.kFBButtonState0, FBColor(0.2, 0.2, 0.2) )
            b.SetStateColor( FBButtonState.kFBButtonState1, FBColor(0.8, 0.2, 0.2) )
            box.Add(b, 35)
        else:
            b.SetStateColor( FBButtonState.kFBButtonState0, FBColor(0.5, 0.5, 0.5) )
            b.SetStateColor( FBButtonState.kFBButtonState1, FBColor(0.8, 0.2, 0.2) )
            b.State = 1
            box.Add(b, 25)
            
        popupControls.append(b)
        
        idx += 1
    #

def PopulateLayout(mainLyt):
    
    global popupControls
    popupControls = []
    
    # create regions
    width = 120
    
    main = FBHBoxLayout()
    
    # Shows how to create a FBVBoxLayout that grows from Top to bottom
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Materials", "Dublicates", "UnUsed", "Delete", ]
    justify = [0,                   2,          2,          2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    #
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Textures", "Dublicates", "Replace Layered", "UnUsed", "Delete"]
    justify = [0,                   2,              2,              2,          2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    #
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Video Clips", "Dublicate", "UnUsed", "Delete"]
    justify = [ 0,                      2,          2,          2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    #
    box = FBVBoxLayout(FBAttachType.kFBAttachTop)
    names = ["Process Shaders", "Delete" ]
    justify = [0,      2]
    
    PopulateVBox(names, justify, box)
    main.Add(box, 120)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachTop,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(200,FBAttachType.kFBAttachNone,"")
    
    mainLyt.AddRegion("main","main", x, y, w, h)
    mainLyt.SetControl("main",main)
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(5,FBAttachType.kFBAttachBottom,"main")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-40,FBAttachType.kFBAttachBottom,"")
    
    mainLyt.AddRegion("console","console", x, y, w, h)
    mainLyt.SetControl("console",gConsole)
    #gConsole.Style = FBListStyle.kFBVerticalList
    cclear()
    
    x = FBAddRegionParam(5,FBAttachType.kFBAttachLeft,"")
    y = FBAddRegionParam(-35,FBAttachType.kFBAttachBottom,"")
    w = FBAddRegionParam(-65,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    mainLyt.AddRegion("doit","doit", x, y, w, h)
    
    # connect controls
    b = FBButton()
    b.Caption = "CleanUp!"
    b.OnClick.Add(ButtonDoItCallback)    
    mainLyt.SetControl("doit", b)

    x = FBAddRegionParam(-60,FBAttachType.kFBAttachRight,"")
    y = FBAddRegionParam(-35,FBAttachType.kFBAttachBottom,"")
    w = FBAddRegionParam(-5,FBAttachType.kFBAttachRight,"")
    h = FBAddRegionParam(-5,FBAttachType.kFBAttachBottom,"")
    
    mainLyt.AddRegion("about","about", x, y, w, h)
    
    # connect controls
    b = FBButton()
    b.Caption = "About"
    b.OnClick.Add(ButtonAboutClickEvent)    
    mainLyt.SetControl("about", b)

        
def CreateTool():    
    
    # Tool creation will serve as the hub for all other controls
    global t
    #t = FBCreateUniqueTool("CleanUp manager " + str(CLEANUP_MANAGER_VERSION) )
    t = FBCreateUniqueTool("CleanUp Manager Tool")
    t.StartSizeX = 530
    t.StartSizeY = 500
    PopulateLayout(t)    
    if gDEVELOPMENT:
        ShowTool(t)


CreateTool()
        
