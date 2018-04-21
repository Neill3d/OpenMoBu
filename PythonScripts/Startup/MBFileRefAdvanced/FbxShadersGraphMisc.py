

# FbxShadersGraphMisc - misc functions
#
# Sergey <Neill3d> Solokhin 2018


import os
import time
import subprocess
import inspect
from pyfbsdk import *


################################ Global Variables

lApp = FBApplication()
lSystem = FBSystem()

gCmdDevPath = 'C:\\Program Files\\Autodesk\\MotionBuilder 2017\\OpenRealitySDK\\samples\\importexport\\FBXExtension\\Debug\\shadingGraph_cmd.exe'
gCmdName = 'shadingGraph_cmd.exe'

gDescClass = 'DescriptionHolder'
gDescAssetPath = 'Browsing/Templates/Elements'
gDescTitle = 'DescriptionHolder'


########################################### Description holder Functions


# Description Rename
#
# \ Return true if any description has been found
#
def DescriptionRename(lNSObj, pNewName):

    lStatus = False
    for i in range(lNSObj.GetDstCount()):
        lPlug = lNSObj.GetDst(i)
        if gDescClass == lPlug.ClassName():
            lPlug.Name = pNewName + '_DescHolder'
            lStatus = True
    
    return lStatus

# DescriptionStore
#   Run a Store action
# setFileName is '' means only Store executed, othewise we set Description FileName property aswell
# \Return True if any description has been found
#
def DescriptionStore(lNSObj, setFileName=''):

    lStatus = False
    lDescObj = None
    
    for i in range(lNSObj.GetDstCount()):
        lPlug = lNSObj.GetDst(i)
        if gDescClass == lPlug.ClassName():
            lDescObj = lPlug
            break
        
    if lDescObj is None:
        # in case we are missing a desc, let's make a new one
        lDescObj = FBCreateObject(gDescAssetPath, gDescTitle, lNSObj.LongName + '_DescHolder')
        
        descProp = lDescObj.PropertyList.Find('Reference Model')
        if descProp is not None:
            FBConnect(lNSObj, descProp)
        descProp = lDescObj.PropertyList.Find('Description FileName')
        if descProp is not None:
            base = os.path.splitext(lNSObj.ReferenceFilePath)[0]
            newXml = base + '.xml'
            descProp.Data = newXml
        
    if lDescObj is not None:        
        if setFileName != '':
            prop = lDescObj.PropertyList.Find('Description FileName')
            if prop is not None:
                prop.Data = setFileName
            
        prop = lDescObj.PropertyList.Find('Store')
        if prop is not None:
            prop()
            
        lStatus = True
        
    return lStatus

# Description Connect More Reference Models

# \ return True if any description has been found

def DescriptionConnectModels(lNSObj, lList):
    
    descProp = None
    for i in range(lNSObj.GetDstCount()):
        lPlug = lNSObj.GetDst(i)
        if gDescClass == lPlug.ClassName():
            descProp = lPlug.PropertyList.Find('Reference Model')
            break
    
    if descProp is not None:
        for name in lList:
            newNS = lSystem.Scene.NamespaceGet(name)
        
            if newNS is not None:
                FBConnect(newNS, descProp) 

    return (descProp is not None)

# Description Get Local FileName
#
# \ Return True if any description has been found

def DescriptionGetLocalFileName(objNS):
    
    xmlname = ''
    origDescFile = ''
    
    for i in range(objNS.GetDstCount()):
        lPlug = objNS.GetDst(i)
        if gDescClass == lPlug.ClassName():
            prop = lPlug.PropertyList.Find('Extract')
            if prop is not None:
                prop()  # Do Extract
                
                prop = lPlug.PropertyList.Find('Temp FileName')
                if prop is not None:
                    xmlname = prop.Data
                    
                prop = lPlug.PropertyList.Find('Description FileName')
                if prop is not None:
                    origDescFile = prop.Data

    return xmlname, origDescFile

# Description Disconnect
#
# \ return True if any description has been found

def DescriptionDisconnect(lNSObj):
    
    lStatus = False
    for i in range(lNSObj.GetDstCount()):
        lPlug = lNSObj.GetDst(i)
        if gDescClass == lPlug.ClassName():
            prop = lPlug.PropertyList.Find('Reference Model')
            
            if prop is not None:
                FBDisconnect(lNSObj, prop)
            
    return lStatus

# Description Delete
#
# \ return True if any description has been found

def DescriptionDelete(lNSObj):
    
    lStatus = False
    if lNSObj is None:
        return lStatus
    
    listToDelete = []
    
    for i in range(lNSObj.GetDstCount()):
        lPlug = lNSObj.GetDst(i)
        if gDescClass == lPlug.ClassName():
            listToDelete.append(lPlug)
            lStatus = True

    [x.FBDelete() for x in listToDelete]
    return lStatus

# return str compatible with fbx MoBuTypeName value
def MBGetShaderTypeName(pShader):
    
    shaderTypeName = 'unknown'
    
    # classname FBShaderLighted
    # reflection - property ReflexionType, ReflexionFactor
    # Particles - property Particule Size, Particule Size Delta

    shaderClass = pShader.ClassName()

    if 'FBShaderLighted' == shaderClass:
        shaderTypeName = 'LightedShader'
    
    elif 'DynamicLighting' == shaderClass:
        shaderTypeName = 'Dynamic Lighting'

    elif 'DynamicLightingShadow' == shaderClass:
        shaderTypeName = 'Dynamic Lighting with Shadows'
    
    elif 'FBShaderShadowLive' == shaderClass:
        shaderTypeName = 'ShadowLiveShader'
    
    elif 'FBShader' == shaderClass:
        bumpShader = pShader.PropertyList.Find('Bump Contrast')
        cgfxShader = pShader.PropertyList.Find('CgFxPath')
        edgeCartoon = pShader.PropertyList.Find('Outline Edges')
        facetedShader = pShader.PropertyList.Find('Enable')
        flatShader = pShader.PropertyList.Find('Enable Luminosity')
        matteShader = pShader.PropertyList.Find('Enable 3D Masking')
        multiCartoon = pShader.PropertyList.Find('Color Steps')
        particleShader = pShader.PropertyList.Find('ParticlesZSort')
        reflectionShader = pShader.PropertyList.Find('Reflection Type')
        selectiveShader = pShader.PropertyList.Find('Selective Lights List')
        shadowMapShader = pShader.PropertyList.Find('Map Size X')
        wireShader = pShader.PropertyList.Find('Line Width')
        
        if bumpShader is not None:
            shaderTypeName = 'BumpShader'  
        elif cgfxShader is not None:
            shaderTypeName = 'CgFx'
        elif edgeCartoon is not None:
            shaderTypeName = 'FlatCartoonShader'
        elif flatShader is not None:
            shaderTypeName = 'FlatShader'
        elif matteShader is not None:
            shaderTypeName = 'ZShader'
        elif multiCartoon is not None:
            shaderTypeName = 'FlatStepShader'
        elif particleShader is not None:
            shaderTypeName = 'ParticuleShader'
        elif reflectionShader is not None:
            shaderTypeName = 'ReflexionShader'
        elif selectiveShader is not None:
            shaderTypeName = 'SelectiveLightingShader'
        elif shadowMapShader is not None:
            shaderTypeName = 'ShadowMapShader'
        elif wireShader is not None:
            shaderTypeName = 'WireShader'
        elif (facetedShader is not None) and (pShader.PropertyList[-1] == facetedShader):
            shaderTypeName = 'FacetedShader'
        
    else:
        shaderTypeName = shaderClass
         
    return shaderTypeName
    
    
################################################################## Fbx ShadersGraph Cmd Functions

def RunCmdExtractXml(filename):
    
    lStatus = False
    
    if os.path.isfile(filename):
    
        cmdPath = ''    
        if os.path.isfile(gCmdDevPath):
            cmdPath = gCmdDevPath
        else:
            lCurFilePath = inspect.currentframe().f_code.co_filename
            cmdPath = os.path.join( os.path.dirname(lCurFilePath), gCmdName )    
    
        if os.path.isfile(cmdPath):
            subprocess.check_call([cmdPath, '-f', filename])
            lStatus = True
            
    return lStatus
    
def RunCmdBake(fbxname, xmlname, outname):
    
    lStatus = False
    
    if os.path.isfile(fbxname) and os.path.isfile(xmlname):
    
        cmdPath = ''    
        if os.path.isfile(gCmdDevPath):
            cmdPath = gCmdDevPath
        else:
            lCurFilePath = inspect.currentframe().f_code.co_filename
            cmdPath = os.path.join( os.path.dirname(lCurFilePath), gCmdName )    
    
        if os.path.isfile(cmdPath):
            subprocess.check_call([cmdPath, '-b', fbxname, xmlname, outname])
            lStatus = True
            
    return lStatus

########################################################### Misc File Functions

def GetFileLastWrite(filename):
    return time.strftime('%d/%m/%Y  %H:%M:%S', time.gmtime(os.path.getmtime(filename)))
    
def ConvertPath(path):
    return os.path.normcase(xmlFileName)
    #return path.replace(os.path.sep, '/')

########################################################## Misc Scene Managment Functions

def CollectReferenceContent(objNS, content):
    
    for i in range(objNS.GetContentCount()):
        comp = objNS.GetContent(i)
        if isinstance(comp, FBNamespace):
            CollectReferenceContent(comp, content)
        else:
            content.append(comp)

def CollectReferenceModels(objNS, content):
    
    for i in range(objNS.GetContentCount()):
        comp = objNS.GetContent(i)
        if isinstance(comp, FBNamespace):
            CollectReferenceModels(comp, content)
        elif isinstance(comp, FBModel):
            content.append(comp)    

def RemoveFirstNamespace(name, namespaceToRemove):
    
    newname = name
    if len(namespaceToRemove) > 0:
        idx = newname.find(namespaceToRemove)
        if 0 == idx:
            newname = newname[len(namespaceToRemove)+1:]
    return newname
    
# find a shader in only connected to namespace models!
def FindAShaderByTags( xmlFileName, refOrigShader, objNS=None ):
    
    lResult = None
    
    for shader in lSystem.Scene.Shaders:
        
        skipShader = False
        if objNS is not None:
            skipShader = True
            for i in range(shader.GetDstCount()):
                lPlug = shader.GetDst(i)
                if objNS == lPlug.GetOwnerFileReference():
                    skipShader = False
                    break
        
        if True == skipShader:
            continue
        
        fnameProp = shader.PropertyList.Find('RefFileName')
        origProp = shader.PropertyList.Find('RefOrigShader')
        
        path1 = os.path.normcase(xmlFileName)
        
        if fnameProp is not None and origProp is not None:
            # DONE: convert to one type of delimeter
            path2 = os.path.normcase(fnameProp.Data)    
            
            if refOrigShader is not None:
            
                if path1 == path2 and refOrigShader == origProp.Data:
                    lResult = shader
                    break
            
            else:
                
                if path1 == path2:
                    lResult = shader
                    break
            
    return lResult

def FindAllShadersByTags( xmlFileName, objNS=None ):
    
    lResult = []
    
    for shader in lSystem.Scene.Shaders:
        
        skipShader = False
        if objNS is not None:
            skipShader = True
            for i in range(shader.GetDstCount()):
                lPlug = shader.GetDst(i)
                if objNS == lPlug.GetOwnerFileReference():
                    skipShader = False
                    break
        
        if True == skipShader:
            continue
        
        fnameProp = shader.PropertyList.Find('RefFileName')        
        path1 = os.path.normcase(xmlFileName)
        
        if fnameProp is not None:

            path2 = os.path.normcase(fnameProp.Data)                
            if path1 == path2:
                lResult.append(shader)
            
    return lResult


def GetPropValueStr(prop):

    value = ""
    try:
        data = prop.Data
        
        if type(data) is int:
            value = str(data)
        elif type(data) is float:
            value = str(data)
        elif type(data) is str:
            value = data
        elif type(data) is bool:
            value = str(data)
        elif type(data) is FBTime:
            value = data.GetTimeString()
        elif type(data) is FBVector2d:
            elem.setAttribute( "X", str(data[0]) )
            elem.setAttribute( "Y", str(data[1]) )
            elem.setAttribute( "Z", str(data[2]) )            
            value = str(data)            
        elif (type(data) is FBVector3d) or (type(data) is FBColor):
            elem.setAttribute( "X", str(data[0]) )
            elem.setAttribute( "Y", str(data[1]) )
            elem.setAttribute( "Z", str(data[2]) )            
            value = str(data)
        elif (type(data) is FBVector4d) or (type(data) is FBColorAndAlpha):
            elem.setAttribute( "X", str(data[0]) )
            elem.setAttribute( "Y", str(data[1]) )
            elem.setAttribute( "Z", str(data[2]) )            
            elem.setAttribute( "W", str(data[3]) )            
            value = str(data)
        else:        
            value = str(data)
            
    except NotImplementedError:
        value = "undefined"

    return value
    
def DisconnectAllShaders(objNS):
    
    lList = []
    CollectReferenceContent(objNS, lList)
    
    for comp in lList:
        if isinstance(comp, FBModel):
            comp.Shaders.removeAll()
        dstCount = comp.GetDstCount()
        for i in range(dstCount):
            plug = comp.GetDst(i)
            
            if isinstance(plug, FBProperty) and isinstance(plug.GetOwner(), FBShader):
                FBDisconnect(comp, plug)
            
def SwapShadersGraph(objNS, oldPath, newPath):
    
    shadersToDelete = []
    
    filename = oldPath
    if filename is None or 0 == len(filename):        
        filename = objNS.ReferenceFilePath
    base = os.path.splitext(filename)[0]
    xmlname = base + '.xml'

    base = os.path.splitext(newPath)[0]
    newXml = base + '.xml'

    # check if new xml is exist
    if False == os.path.isfile(newXml):
        lOption = FBMessageBox('Reference Tool', 'Shaders Graph for a new file path is missing\n Do you want to create a new one?', 'Yes', 'Cancel')
        if 1 == lOption:
            if os.path.isfile(newPath):
                RunCmdExtractXml(newPath)

    # TODO: collect namespace connected shaders
    
    refShaders = FindAllShadersByTags(xmlname)

    for lShader in refShaders:
        propFileName = lShader.PropertyList.Find('RefFileName')
        if propFileName is not None:
            propFileName.Data = newXml
    
    # holder
    DescriptionStore(objNS, newXml)
            
def DeleteShadersGraph(objNS, referenceFilePath=''):
    
    shadersToDelete = []
    
    filename = referenceFilePath
    if filename is None or 0 == len(filename):
        if objNS is not None:
            filename = objNS.ReferenceFilePath
            
    if 0 == len(filename):
        return
            
    base = os.path.splitext(filename)[0]
    xmlname = base + '.xml'
    
    #
    shadersToDelete = FindAllShadersByTags(xmlname)
    
    #
    [x.FBDelete() for x in shadersToDelete]