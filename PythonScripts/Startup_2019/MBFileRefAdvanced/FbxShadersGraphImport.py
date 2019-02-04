
#
# Shaders Graph Module
#
# Sergei <Neill3d> Solokhin 2018
#
# Special for Les Androids Associes

import os
import inspect
import sys

from pyfbsdk import *

from xml.dom import minidom

import FbxShadersGraphMisc as misc

lApp = FBApplication()
lSystem = FBSystem()

# stores a list of content components for the current reference file
gReferenceContent = []
gReferenceContentNames = []
    
###################################################################################
########################################### Shaders

def LookForAPropName(allThePropElems, key):
    
    for propelem in allThePropElems:
        propName = str(propelem.getAttribute('Name'))
        
        if key == propName:
            return True
            
    return False



def CreateShader(shaderClass, shaderName, allThePropElems):

    # classname FBShaderLighted
    # reflection - property ReflexionType, ReflexionFactor
    # Particles - property Particule Size, Particule Size Delta

    newShader = None
    #print shaderClass
    if 'LightedShader' == shaderClass:
        newShader = FBShaderLighted(shaderName) 
    
    elif 'Dynamic Lighting' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Dynamic Lighting", shaderName)            
        lSystem.Scene.Shaders.append(newShader)

    elif 'Dynamic Lighting with Shadows' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Dynamic Lighting & Shadow", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'ShadowLiveShader' == shaderClass:
        newShader = FBShaderShadowLive(shaderName)
    
    elif 'ReflexionShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Reflection", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'ParticuleShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Particle", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'FacetedShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Faceted", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
     
    elif 'BumpShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Bump Map", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'SelectiveLightingShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Selective Lighting", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'ShadowMapShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Shadow Map", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'FlatCartoonShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Edge Cartoon", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'FlatStepShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Multilevel Cartoon", shaderName)            
        lSystem.Scene.Shaders.append(newShader) 
    
    elif 'WireShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Wire Frame", shaderName)            
        lSystem.Scene.Shaders.append(newShader)  
    
    elif 'FlatShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Flat", shaderName)            
        lSystem.Scene.Shaders.append(newShader)
    
    elif 'ZShader' == shaderClass:
        newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "Matte", shaderName)            
        lSystem.Scene.Shaders.append(newShader)   
    
    else:
        
        cgfxShader = LookForAPropName(allThePropElems, 'CgFxPath')
        
        if cgfxShader:
            newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", "CgFxPath", shaderName)            
            lSystem.Scene.Shaders.append(newShader)
        else:
            newShader = FBCreateObject("Browsing/Templates/Shading Elements/Shaders", shaderClass, shaderName)            
            if newShader is not None:
                lSystem.Scene.Shaders.append(newShader)            
            
            
    return newShader

def RetrieveXmlPropertyValue(propElem):
    
    propType = str(propElem.getAttribute('Type'))
    propType = propType.lower()
    propValue = str(propElem.getAttribute('Value'))    

    lResult = None
    
    if 'bool' == propType:
        lResult = ('True' == propValue)
    elif 'number' == propType or 'double' == propType:
        lResult = float(propValue)
    elif 'integer' == propType or 'int' == propType:
        lResult = int(propValue)
    elif 'enum' == propType:
        lResult = int(propValue)
    elif 'charptr' == propType:
        lResult = propValue
    elif 'vector2d' == propType:
        x = str(propElem.getAttribute('X'))
        y = str(propElem.getAttribute('Y'))
        lResult = FBVector2d(float(x), float(y))
    elif 'vector' == propType:
        x = str(propElem.getAttribute('X'))
        y = str(propElem.getAttribute('Y'))
        z = str(propElem.getAttribute('Z'))
        
        lResult = FBVector3d(float(x), float(y), float(z))
        
    elif 'vector4d' == propType:
        x = str(propElem.getAttribute('X'))
        y = str(propElem.getAttribute('Y'))
        z = str(propElem.getAttribute('Z'))
        w = str(propElem.getAttribute('W'))
                    
        lResult= FBVector4d(float(x), float(y), float(z), float(w))
        
    elif 'color' == propType or 'colorrgb' == propType:
        x = str(propElem.getAttribute('X'))
        y = str(propElem.getAttribute('Y'))
        z = str(propElem.getAttribute('Z'))
        
        lResult = FBColor(float(x), float(y), float(z))
        
    elif 'colorandalpha' == propType or 'colorrgba' == propType:
        x = str(propElem.getAttribute('X'))
        y = str(propElem.getAttribute('Y'))
        z = str(propElem.getAttribute('Z'))
        w = str(propElem.getAttribute('W'))
        
        lResult = FBColorAndAlpha(float(x), float(y), float(z), float(w))
        
    return lResult

def GetXmlProps(allThePropElems):
    
    props = []
    
    for propElem in allThePropElems:
        propName = str(propElem.getAttribute('Name'))
        
        numberOfConnections = 0
        connectionRoot = propElem.getElementsByTagName('Connections')

        if len(connectionRoot) > 0:
            numberOfConnections = int(connectionRoot[0].getAttribute('Count'))

        props.append([propName, RetrieveXmlPropertyValue(propElem), numberOfConnections])
    
    return props
    
def GetXmlAttachments(allTheAttElems, prefix):
    
    att = []
    
    for attachment in allTheAttElems:
        modelname = str(attachment.getAttribute('LongName'))
        att.append(prefix + modelname)
    
    return att

def GetMBAttachments(lShader, objNS):
    
    att = []
    
    for i in range(lShader.GetDstCount()):
        lPlug = lShader.GetDst(i)
        if not isinstance(lPlug, FBScene):

            if lPlug in gReferenceContent:
                att.append( lPlug.LongName )
        
    return att

def GlobalCollectReferenceContent(objNS):
    
    for i in range(objNS.GetContentCount()):
        comp = objNS.GetContent(i)
        if isinstance(comp, FBNamespace):
            GlobalCollectReferenceContent(comp)
        else:
            gReferenceContent.append(comp)
            gReferenceContentNames.append(comp.LongName)

def RetrieveShaderPropertyValue(lShader, propElem, importNS):
    
    propName = str(propElem.getAttribute('Name'))

    theProp = lShader.PropertyList.Find(propName)
    if theProp is not None:
        #sys.__stdout__.write('theProp Name - ' + theProp.GetName() + '\n')
        
        newPropValue = RetrieveXmlPropertyValue(propElem)
        if newPropValue is not None:
            theProp.Data = newPropValue
        
        # check for connections
        theProp.DisconnectAllSrc()
        
        connectionRoot = propElem.getElementsByTagName('Connections')
        if len(connectionRoot) > 0:
       
            count = int(connectionRoot[0].getAttribute('Count'))
            if count > 0:
                
                allTheConnections = connectionRoot[0].getElementsByTagName('Source')
                for connElem in allTheConnections:
                    compName = importNS + str(connElem.getAttribute('LongName'))

                    ndx = gReferenceContentNames.index(compName)
                    if ndx >= 0:
                        #theProp.ConnectSrc(comp)
                        FBConnect(gReferenceContent[ndx], theProp)
                        #sys.__stdout__.write(gReferenceContent[ndx].LongName + '\n')

#
# findByTag is bool
# alwaysRetrieveProps is bool
#

def LoadShaderGraphResources(filename, origFileName, objNS, findByTag, alwaysRetrieveProps):

    importNS = ''    
    shaderNS = ''
    if objNS is not None:
        importNS = objNS.LongName + ':'
        shaderNS = objNS.LongName + 'S:'
    
    global gReferenceContent
    global gReferenceContentNames
    
    gReferenceContent = []
    gReferenceContentNames = []
    
    GlobalCollectReferenceContent(objNS)
    
    xmldoc = minidom.parse(filename)
    shadersGraphElem = xmldoc.getElementsByTagName('Shader')
    
    if len(shadersGraphElem) > 0:
        for shader in shadersGraphElem:
        
            shaderClass = str(shader.getAttribute('ClassName'))
            shaderName = str(shader.getAttribute('LongName'))
            isSystem = ('True' == str(shader.getAttribute('System')))
            
            # SHADER AND PROPERTIES
            allTheAttElems = shader.getElementsByTagName('Dst')
            allThePropElems = shader.getElementsByTagName('Property') 

            newShader = None
            
            if True == isSystem:
                newShader = FBFindObjectByFullName('Shader::' + shaderName)
            else:
                if True == findByTag:
                    newShader = misc.FindAShaderByTags( origFileName, shaderName )
                else:
                    newShader = FBFindObjectByFullName('Shader::' + shaderName)
            
            if newShader is None:                           
                newShader = CreateShader(shaderClass, shaderNS + shaderName, allThePropElems)
                
                if False == alwaysRetrieveProps:
                    if newShader is not None:
                        for prop in allThePropElems:
                            RetrieveShaderPropertyValue(newShader, prop, importNS)
                    
                    # add props for shader identification
                    fnameProp = newShader.PropertyCreate( 'RefFileName', FBPropertyType.kFBPT_charptr, 'String', False, True, None )
                    origProp = newShader.PropertyCreate( 'RefOrigShader', FBPropertyType.kFBPT_charptr, 'String', False, True, None )
    
                    if fnameProp is not None:
                        fnameProp.Data = origFileName
                    if origProp is not None:
                        origProp.Data = shaderName
    
                    # connections
                    if newShader is not None:
                        
                        for attachment in allTheAttElems:
                            modelname = str(attachment.getAttribute('LongName'))
                            
                            ndx = gReferenceContentNames.index(importNS + modelname)
                            if ndx >= 0:
                                FBConnect(newShader, gReferenceContent[ndx])
            
            #
            if newShader is not None:
                
                if True == isSystem:
                    for attachment in allTheAttElems:
                        modelname = str(attachment.getAttribute('LongName'))
                        ndx = gReferenceContentNames.index(importNS + modelname)
                        if ndx >= 0:
                            FBConnect(newShader, gReferenceContent[ndx])
                else:
                
                    if True == alwaysRetrieveProps:
                        
                        for prop in allThePropElems:
                            RetrieveShaderPropertyValue(newShader, prop, importNS)
                            
                        for attachment in allTheAttElems:
                            modelname = str(attachment.getAttribute('LongName'))
                            ndx = gReferenceContentNames.index(importNS + modelname)
                            if ndx >= 0:
                                FBConnect(newShader, gReferenceContent[ndx])
  
                        # id props
                        fnameProp = newShader.PropertyList.Find( 'RefFileName' )
                        if fnameProp is None:
                            fnameProp = newShader.PropertyCreate( 'RefFileName', FBPropertyType.kFBPT_charptr, 'String', False, True, None )
                        origProp = newShader.PropertyList.Find( 'RefOrigShader' )
                        if origProp is None:
                            origProp = newShader.PropertyCreate( 'RefOrigShader', FBPropertyType.kFBPT_charptr, 'String', False, True, None )
        
                        if fnameProp is not None:
                            fnameProp.Data = origFileName
                        if origProp is not None:
                            origProp.Data = shaderName
        #
    #del (xmldoc)

def LoadShaderGraphConnections(filename, origFileName, objNS, findByTag):

    importNS = ''    
    if objNS is not None:
        importNS = objNS.LongName + ':'

    global gReferenceContent
    global gReferenceContentNames
    
    gReferenceContent = []
    gReferenceContentNames = []
    
    GlobalCollectReferenceContent(objNS)

    xmldoc = minidom.parse(filename)
    shadersGraphElem = xmldoc.getElementsByTagName('Shader')
    
    if len(shadersGraphElem) > 0:
        for shader in shadersGraphElem:
        
            shaderClass = str(shader.getAttribute('ClassName'))
            shaderName = str(shader.getAttribute('LongName'))
            strSystem = str(shader.getAttribute('System'))
            
            # SHADER AND PROPERTIES
            allTheAttElems = shader.getElementsByTagName('Dst')
            allThePropElems = shader.getElementsByTagName('Property') 

            newShader = None
            
            if 'True' == strSystem:
                for lShader in lSystem.Scene.Shaders:
                    if lShader.HasObjectFlags(FBObjectFlag.kFBFlagSystem) or 'Default Shader' == lShader.Name:
                        newShader = lShader
                        break
            else:
                if True == findByTag:
                    newShader = misc.FindAShaderByTags( origFileName, shaderName )
                else:
                    newShader = FBFindObjectByFullName('Shader::' + shaderName)                

            # connections
            if newShader is not None:
                
                for prop in allThePropElems:
                    RetrieveShaderPropertyValue(newShader, prop, importNS)
                            
                for attachment in allTheAttElems:
                    modelname = str(attachment.getAttribute('LongName'))
                    ndx = gReferenceContentNames.index(importNS + modelname)
                    if ndx >= 0:
                        FBConnect(newShader, gReferenceContent[ndx])
            
    #
    # import model visibility and show
    
    modelsElem = xmldoc.getElementsByTagName('Models')
    
    if len(modelsElem) > 0:
        
        allTheModels = modelsElem[0].getElementsByTagName('Model')
        
        for modelelem in allTheModels:
        
            modelName = str(modelelem.getAttribute('LongName'))
            show = ('True' == str(modelelem.getAttribute('Show')))
            vis = ('True' == str(modelelem.getAttribute('Visibility')))
            
            ndx = gReferenceContentNames.index(importNS + modelName)
            if ndx >= 0:
                model = gReferenceContent[ndx]
                
                currShow = model.Show
                if currShow != show:
                    model.Show = show
                    
                currVis = model.Visibility.Data
                if currVis != vis:
                    model.Visibility = vis
                
                if False == show or False == vis:
                    print modelName
    
    #del (xmldoc, shadersGraphElem, modelsElem)

# mbObj to compare with PropertyList
# propData is a couple [name, type, value]

# sets could be with different types, we return an error - not the same Classes!
# sets could miss some props, it doesn't matter, we are comparing existing one


# returns props info : number of shared, number of edits, number of collisions

def ComparePropsAndValues(mbObj, props):
    
    numberOfSharedProps = 0
    numberOfPropEdits = 0
    numberOfPropCollisions = 0  # if types are not the same !
    
    # prop is a couple [name, value]
    for prop in props:
        mbProp = mbObj.PropertyList.Find(prop[0])
        
        if mbProp is not None:
            numberOfSharedProps += 1
            
            isCollided = False
            isEdited = False
            
            try:
                mbPropValue = mbProp.Data
                propValue = prop[1]
                
                if type(mbPropValue) == type(propValue):
                    
                    if float == type(mbPropValue):
                        if abs(mbPropValue-propValue) > 0.001:
                            isEdited = True
                    else:
                        if mbPropValue != propValue:
                            isEdited = True
                        
                else:
                    isCollided = True
                    
            except NotImplementedError:
                #numberOfPropCollisions += 1
                print mbProp.GetName()
        
            # TODO: check prop connections if there are any
            srcCount = mbProp.GetSrcCount()
            if srcCount != prop[2]:
                isEdited = True
            
            if True == isEdited:
                numberOfPropEdits += 1
            if True == isCollided:
                numberOfPropCollisions += 1
            
    return [numberOfSharedProps, numberOfPropEdits, numberOfPropCollisions]

# make a mask of user made changes

def ComputeShadersMask(objNS):

    mask = []

    importNS = objNS.LongName + ':'
    filename = objNS.ReferenceFilePath
    
    # look to description holder at first
    xmlname, origFName = misc.DescriptionGetLocalFileName(objNS)
    
    if xmlname == '' or False == os.path.isfile(xmlname):
        
        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'
    
    if os.path.isfile(xmlname):
        
        xmldoc = minidom.parse(xmlname)
        shadersGraphElem = xmldoc.getElementsByTagName('Shader')

# DONE: load a description file and compare with scene shaders !
# objNS - FBFileReference model
# sceneShaders - current connected shaders to the NS model
# return a str compare description 
def CompareShadersGraph(objNS, sceneShaders):
    
    global gReferenceContent
    global gReferenceContentNames
    
    gReferenceContent = []
    gReferenceContentNames = []
    
    GlobalCollectReferenceContent(objNS)
    
    text = ''
    
    importNS = objNS.LongName + ':'
    filename = objNS.ReferenceFilePath
    
    # look to description holder at first
    xmlname, origFName = misc.DescriptionGetLocalFileName(objNS)
    
    if xmlname == '' or False == os.path.isfile(xmlname):
        
        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'
    
    if os.path.isfile(xmlname):
        
        numberOfExistShaders = 0
        existShaders = []
        for shader in sceneShaders:
            existShaders.append(False)
        
        xmldoc = minidom.parse(xmlname)
        shadersGraphElem = xmldoc.getElementsByTagName('Shader')
        
        text = text + '== Parsing Shaders Graph Description - ' + xmlname + '\n'
        
        if len(shadersGraphElem) > 0:
            for shader in shadersGraphElem:
            
                shaderClass = str(shader.getAttribute('ClassName'))
                xmlShaderName = str(shader.getAttribute('LongName'))
                strSystem = str(shader.getAttribute('System'))
                
                text = text + '\n  = Shader ' + xmlShaderName + ' (Class: ' + shaderClass + ') ... '
                
                # DONE: situation when scene shader has been renamed (I'm using RefOrigShader Property)
                
                isExists = False
                lSceneShader = None
                                
                if 'True' == strSystem:
                    
                    for lShader in lSystem.Scene.Shaders:
                        if lShader.HasObjectFlags(FBObjectFlag.kFBFlagSystem) or 'Default Shader' == lShader.Name:
                            lSceneShader = lShader
                            isExists = True
                            break
                else:    

                    for i, sceneShader in enumerate(sceneShaders):
                        mbShaderName = sceneShader.LongName
                        refOrigShader = sceneShader.PropertyList.Find('RefOrigShader')
                        if refOrigShader is not None:
                            mbShaderName = refOrigShader.Data
                            
                        if xmlShaderName == mbShaderName:
                            lSceneShader = sceneShader
                            existShaders[i] = True
                            isExists = True
                            break
                        
                if isExists:
                    numberOfExistShaders += 1
                    
                    text = text + ' Ok!\n'
                else:
                    text = text + ' Missing!\n'
                
                
                allTheAttElems = shader.getElementsByTagName('Dst')
                allThePropElems = shader.getElementsByTagName('Property')
                
                # DONE: compare prop values
                
                if lSceneShader is not None:
                    
                    # DONE: we could not have a scene load for retreiving mod state
                    xmlprops = []
                    xmlprops = GetXmlProps(allThePropElems)
                
                    # shared, edits, collisions
                    compareInfo = []
                    compareInfo = ComparePropsAndValues(lSceneShader, xmlprops)
                
                    text = text + '   > Checking Properties - total: ' + str(len(lSceneShader.PropertyList))
                    text = text + '; shared: ' + str(compareInfo[0]) + '; edits: ' + str(compareInfo[1]) + '; collisions: ' + str(compareInfo[2]) + '; \n'
                                    
                    # DONE: compare attachments
                    
                    att = []
                    att = GetXmlAttachments(allTheAttElems, importNS)
                    mbAtt = []
                    mbAtt = GetMBAttachments(lSceneShader, objNS)
                    
                    diff1 = list(set(mbAtt) - set(att))
                    diff2 = list(set(att) - set(mbAtt))
                    
                    diff = diff1 + diff2
                    
                    if lSceneShader.HasObjectFlags(FBObjectFlag.kFBFlagSystem):
                        diff = diff2
                                        
                    if len(diff) > 0:
                        text = text + '    > Different shader-model assignment - '
                        for elem in diff:
                            text = text + elem + '; '
                        text = text + '\n'
                    else:
                        text = text + '    > No differents in shader-model assignments!\n'
        
        if numberOfExistShaders == len(sceneShaders):
            
            if len(shadersGraphElem) == len(sceneShaders):            
                text = text + '\nThere are no changes in shaders\n'
            else:
                text = text + '\nSome Source Description shaders are missing (Disconnected or Removed)\n'
        else:
            
            text = text + '\nSome Scene Shaders are missing (User Added or Connected)\n'
            
            for lFlag, lShader in zip(existShaders, sceneShaders):
                if False == lFlag:
                    text = text + '  * ' + lShader.LongName + '\n'
            
                   
    return text

# silent True to not show a dialog
# importMode 1 for apply to ref, 2 for apply to source
# objNS - reference namespace object
# modelsMask - a list of new appended models, so we mask connection changes

def DoFbxShadersGraphImport(silent, importMode, objNS, modelsMask):

    userChoise = 1 #importMode
    
    if not silent:
        userChoise = FBMessageBox('Shader Graph Importer', 'Please choose an operation', 'Apply to reference', 'Apply to source scene', 'Cancel')
    
    if 1 == userChoise:
        
        # auto load for references
        
        filename = objNS.ReferenceFilePath

        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'

        # TODO: if description file is not exist, let's try to make it from fbx
        # TODO: xml will be update if it's out of date
        
        if os.path.isfile(filename):
            misc.RunCmdExtractXml(filename)

        #
        #
        if os.path.isfile(xmlname):
            
            # step 1 - load missing resources
            #   ?! promt if user wants to revert shaders properties values

            LoadShaderGraphResources(xmlname, xmlname, objNS, True, (importMode > 1))

            # step 2 - apply connections and modifications (on top of last user edits)
            
            LoadShaderGraphConnections(xmlname, xmlname, objNS, True)
            
        else:
            FBMessageBox('Shading Graph Importer', 'Shading Graph Description is not found !', 'Ok')
    
    elif 2 == userChoise:
        
        # we don't have reference namespace in a source scene
        
        filename = lApp.FBXFileName
        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'
        
        if os.path.isfile(xmlname):
            # apply changes to orig scene
            LoadShaderGraphResources(xmlname, xmlname, None, False, True)
            LoadShaderGraphConnections(xmlname, xmlname, None, False)
        else:
            FBMessageBox('Shading Graph Importer', 'Shading Graph Description is not found!', 'Ok')
    

def DoFbxShadersGraphImport2(silent, importMode, objNS, modelsMask):

    userChoise = 1 #importMode
    
    if not silent:
        userChoise = FBMessageBox('Shader Graph Importer', 'Please choose an operation', 'Apply to reference', 'Apply to source scene', 'Cancel')
    
    if 1 == userChoise:
        
        # auto load for references
        
        NS = objNS.LongName + ':'
        filename = objNS.ReferenceFilePath

        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'

        # DONE: if description file is not exist, let's try to make it from fbx
        # DONE: xml will be update if it's out of date
        
        if os.path.isfile(filename):
            misc.RunCmdExtractXml(filename)

        #
        #
        
        if os.path.isfile(xmlname):
            
            refEdits = ''
            #if objNS.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):            
            #    refEdits = objNS.GetRefEdit()
            #    objNS.RevertRefEdit()
            
            # step 1 - load missing resources
            #   ?! promt if user wants to revert shaders properties values

            LoadShaderGraphResources(xmlname, xmlname, NS, True, (importMode > 1))

            # step 2 - apply connections and modifications (on top of last user edits)
            
            if len(refEdits) > 0:
                objNS.RevertRefEdit()
                objNS.ApplyRefEditPyScriptFromString(refEdits)
                LoadShaderGraphConnections(xmlname, xmlname, objNS, True)
                # restore missing connections, like when merging new objects
            else:
                userChoise = 1 #FBMessageBox('Shading Graph Importer', 'Do you want to restore shaders graph connections?', 'Ok', 'Cancel')
                if 1 == userChoise:
                    LoadShaderGraphConnections(xmlname, xmlname, objNS, True)
            
            #del(refEdits)
            
        else:
            FBMessageBox('Shading Graph Importer', 'Shading Graph Description is not found !', 'Ok')
    
    elif 2 == userChoise:
        
        # we don't have reference namespace in a source scene
        
        filename = lApp.FBXFileName
        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'
        
        if os.path.isfile(xmlname):
            # apply changes to orig scene
            LoadShaderGraphResources(xmlname, xmlname, None, False, True)
            LoadShaderGraphConnections(xmlname, xmlname, None, False)
        else:
            FBMessageBox('Shading Graph Importer', 'Shading Graph Description is not found!', 'Ok')
    

def RestoreShadersGraph(objNS):
    
    xmlname, origDescFile = misc.DescriptionGetLocalFileName(objNS)
    
    if xmlname == '' or False == os.path.isfile(xmlname):
        filename = objNS.ReferenceFilePath
        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'
        origDescFile = xmlname
       
    #DeleteShadersGraph(objNS)
    if os.path.isfile(xmlname):
        modelsMask = []
        LoadShaderGraphResources(xmlname, origDescFile, objNS, True, True)
        LoadShaderGraphConnections(xmlname, origDescFile, objNS, True)
    
def HasReferenceNewUpdate(objNS):
    
    filename = objNS.ReferenceFilePath
    date = misc.GetFileLastWrite(filename)
    
    # check for xml last write
    xmlname, origFName = misc.DescriptionGetLocalFileName(objNS)
                    
    if '' == xmlname:
        lOption = FBMessageBox( 'Reference Tool', 'Description Holder is missing for ' + objNS.LongName, 'Ok', 'Create A New' )
        
        if 2 == lOption:
            misc.DescriptionStore(objNS)
            
        return False
        
    xmldoc = minidom.parse(xmlname)
    shadersGraphElem = xmldoc.getElementsByTagName('ShadersGraph')
    if 0 == len(shadersGraphElem):
        return False
    
    xmlLastWrite = str(shadersGraphElem[0].getAttribute('LastWrite'))
    
    print date
    print xmlLastWrite
    
    return (date > xmlLastWrite)