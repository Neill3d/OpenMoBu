
#
# Shaders Graph Exporter
#
# Sergey <Neill3d> Solokhin 2018
#
# function to bake - BakeShadersGraphEdits(objNS, xmlname)

import os
import time
import subprocess

from pyfbsdk import *
from xml.dom import minidom

import FbxShadersGraphMisc as misc

######################################### Global Variables

lApp = FBApplication()
lSystem = FBSystem()

######################################### Functions

def StoreProperty(newdoc, top_element, prop, namespaceToRemove):
    
    if prop.GetPropertyFlag(FBPropertyFlag.kFBPropertyFlagHideProperty):
        return 0
    if prop.IsReferenceProperty():
        return 0
    
    propname = prop.Name
    if 'RefFileName' == propname or 'RefOrigShader' == propname:
        return 0
    
    elem = newdoc.createElement('Property')
    elem.setAttribute( 'Name', propname )
    elem.setAttribute( 'Type', str(prop.GetPropertyTypeName()) )
    
    value = ""
    try:
        data = prop.Data
        
        if type(data) is FBTime:
            value = data.GetTimeString()
        elif type(data) is FBVector2d:
            elem.setAttribute( "X", str(data[0]) )
            elem.setAttribute( "Y", str(data[1]) )
            elem.setAttribute( "Z", str(data[2]) )            
            value = str(data)            
        elif type(data) in [FBVector3d, FBColor]:
            elem.setAttribute( "X", str(data[0]) )
            elem.setAttribute( "Y", str(data[1]) )
            elem.setAttribute( "Z", str(data[2]) )            
            value = str(data)
        elif type(data) in [FBVector4d, FBColorAndAlpha]:
            elem.setAttribute( "X", str(data[0]) )
            elem.setAttribute( "Y", str(data[1]) )
            elem.setAttribute( "Z", str(data[2]) )            
            elem.setAttribute( "W", str(data[3]) )            
            value = str(data)
        else:        
            value = str(data)
            
    except NotImplementedError:
        value = "undefined"
    except Exception:
        value = "undefined"
        
    elem.setAttribute( "Value", value )
    top_element.appendChild(elem)
    
    # write property connectors (models, textues, etc. like FBPropertyObjectList)
    srcCount = prop.GetSrcCount()
    if srcCount > 0:
        conns_elem = newdoc.createElement('Connections')
        conns_elem.setAttribute('Count', str(srcCount) )
        elem.appendChild(conns_elem)
        
        for i in range(srcCount):
            comp = prop.GetSrc(i)
    
            src_elem = newdoc.createElement('Source')        
            conns_elem.appendChild(src_elem)
            
            longname = comp.LongName
            longname = misc.RemoveFirstNamespace(longname, namespaceToRemove)
            
            src_elem.setAttribute('ClassName', comp.ClassName() )
            src_elem.setAttribute('Name', comp.Name)
            src_elem.setAttribute('LongName', longname )
    
    return 1

#
# StoreShader 

def StoreShader(newdoc, top_element, shader, namespaceToRemove):
    
    shader_elem = newdoc.createElement('Shader')
    shader_elem.setAttribute( 'ClassName', misc.MBGetShaderTypeName(shader) )
    shader_elem.setAttribute( 'Name', shader.Name )
    shader_elem.setAttribute( 'LongName', misc.RemoveFirstNamespace(shader.LongName, namespaceToRemove+'S') )
    shader_elem.setAttribute( 'System', str(shader.HasObjectFlags(FBObjectFlag.kFBFlagSystem)) )
    
    top_element.appendChild(shader_elem)
    
    # Attachments
    atts_elem = newdoc.createElement('Attachments')
    
    attCount = 0
    for i in range(shader.GetDstCount()):
        comp = shader.GetDst(i)
        
        if False == isinstance(comp, FBScene):
            attCount += 1
    
    atts_elem.setAttribute( 'Count', str(attCount) )
    shader_elem.appendChild(atts_elem)

    for i in range(shader.GetDstCount()):
        comp = shader.GetDst(i)
        
        if isinstance(comp, FBScene):
            continue
        
        dst_elem = newdoc.createElement('Dst')
        atts_elem.appendChild(dst_elem)
        
        longname = comp.LongName
        longname = misc.RemoveFirstNamespace(longname, namespaceToRemove)
        
        dst_elem.setAttribute('ClassName', comp.ClassName() )
        dst_elem.setAttribute('Name', comp.Name )
        dst_elem.setAttribute('LongName', longname )
    
    # Properties
    
    props_elem = newdoc.createElement('Properties')
    shader_elem.appendChild(props_elem)
    
    numberOfExported = 0
    for prop in shader.PropertyList:
        status = StoreProperty(newdoc, props_elem, prop, namespaceToRemove)
        numberOfExported += status
    
    props_elem.setAttribute('Count', str(numberOfExported))

#
# StoreModel

def StoreModel(newdoc, top_element, model, namespaceToRemove):

    longname = model.LongName
    longName = misc.RemoveFirstNamespace(longname, namespaceToRemove)

    model_elem = newdoc.createElement('Model')
    model_elem.setAttribute( 'ClassName', model.ClassName() )
    model_elem.setAttribute( 'Name', model.Name )
    model_elem.setAttribute( 'LongName', longName )
    model_elem.setAttribute( 'Visibility', str(model.Visibility) )
    model_elem.setAttribute( 'Show', str(model.Show) )
    
    top_element.appendChild(model_elem)

#
# Save Shader Graph

def SaveShaderGraph(filename, FBXFileName, shadersList, modelsList, namespaceToRemove):
    
    impl = minidom.getDOMImplementation()
    
    newdoc = impl.createDocument(None, 'ShadersGraph', None)
    top_element = newdoc.documentElement    
    top_element.setAttribute('FileName', FBXFileName)
    
    # TODO: put file size and file last write
    size = os.path.getsize(FBXFileName)
    date = misc.GetFileLastWrite(FBXFileName)
    
    top_element.setAttribute("FileSize", str(size))
    top_element.setAttribute("LastWrite", str(date))
    
    shaders_elem = newdoc.createElement('Shaders')
    shaders_elem.setAttribute('Count', str(len(shadersList)))
    top_element.appendChild(shaders_elem)
    
    for shader in shadersList:
        StoreShader(newdoc, shaders_elem, shader, namespaceToRemove)
    
    models_elem = newdoc.createElement('Models')
    models_elem.setAttribute('Count', str(len(modelsList)))
    top_element.appendChild(models_elem)
    
    for comp in modelsList:
        StoreModel(newdoc, models_elem, comp, namespaceToRemove)
    
    res = open(filename, 'w')
    res.writelines(newdoc.toprettyxml())
    res.close()    

#
# BakeShadersGraphEdits

def BakeShadersGraphEdits(objNS, xmlname):

    if objNS is None:
        return False
    if False == os.path.isfile(xmlname):
        #raise NameError('Shaders Graph description is not found!')
        print ('Shaders Graph description is not found!')
        return False


    shadersList = []
    modelsList = []

    filename = objNS.ReferenceFilePath

    #shadersList = misc.FindAllShadersByTags(xmlname, objNS)
    misc.CollectReferenceModels(objNS, modelsList)
  
    for model in modelsList:
        for shader in model.Shaders:
            
            if not (shader in shadersList):
                shadersList.append(shader)
  
    # write a new one
    SaveShaderGraph(xmlname, objNS.ReferenceFilePath, shadersList, modelsList, objNS.LongName)

    # 1. bake updates into ref file
    
    misc.RunCmdBake(objNS.ReferenceFilePath, xmlname, objNS.ReferenceFilePath)
    
    # 2. store a new description holder - make baked state as initial
    misc.DescriptionStore(objNS)

    #except NameError:
    #    FBMessageBox( 'Shaders Graph', 'Reference object or shaders are not found!', 'Ok')    
    return True
