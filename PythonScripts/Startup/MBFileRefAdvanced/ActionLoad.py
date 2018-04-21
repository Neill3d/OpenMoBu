
# Internal action for reference manager
#
# Sergey <Neill3d> Solokhin 2018

from pyfbsdk import *

import inspect
import sys
import os

lCurFilePath = inspect.currentframe().f_code.co_filename
sys.path.append( os.path.dirname(lCurFilePath) )
#sys.path.append( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )

import FbxShadersGraphImport as imp
import FbxShadersGraphMisc as misc
lRefName = ''
lSystem = FBSystem()
lScene = lSystem.Scene

lNSObj = None

if len(lRefName) > 0:
    
    for ns in lScene.Namespaces:
        if isinstance(ns, FBFileReference) and lRefName == ns.LongName:
            if False == os.path.isfile(ns.ReferenceFilePath):
                FBMessageBox('Reference A File', 'Referenced file is not found!', 'Ok')
            else:            
                lNSObj = ns
            break
        
if lNSObj is not None:
    filename = lNSObj.ReferenceFilePath
    isUpdated = imp.HasReferenceNewUpdate(lNSObj)
    lOption = 3
    
    if isUpdated:
        # ask user to reload a shaders graph
        lOption = FBMessageBox( "External File Changed", "Please choose the following action for Reference: %s!" % ( filename ), "Load", "Merge", "Ignore" )                        
    
    if lOption <> 3:
        lUndo = FBUndoManager()
        lUndo.Clear()
        
        modelsMask = []
        
        edits = ''
        misc.DisconnectAllShaders(lNSObj)
        if lNSObj.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
            edits = lNSObj.GetRefEdit()
        
        lNSObj.IsLoaded = False
        lNSObj.IsLoaded = True
        
        filename = lNSObj.ReferenceFilePath
    
        base = os.path.splitext(filename)[0]
        xmlname = base + '.xml'
        
        if lOption == 2:
            
            imp.DoFbxShadersGraphImport(True, 2, lNSObj, modelsMask)
            
            if len(edits) > 0:
                lNSObj.RevertRefEdit()
                lNSObj.ApplyRefEditPyScriptFromString( edits )
                
            imp.LoadShaderGraphConnections(xmlname, xmlname, lNSObj, True)

        else:
            lNSObj.RevertRefEdit()
            imp.DoFbxShadersGraphImport(True, 2, lNSObj, modelsMask)
        
        # store new xml data
        misc.DescriptionStore(lNSObj)

    else:
        #lNSObj.IsLoaded = True
    
        if True == lNSObj.IsLoaded:
            prop = lNSObj.PropertyList.Find('FirstTime')
            if prop is not None:
                modelsMask = []
                imp.DoFbxShadersGraphImport(True, 2, lNSObj, modelsMask)
                lNSObj.PropertyRemove(prop)
        else:
            FBMessageBox(self.DialogTitle, 'Failed to reload a reference', 'Ok')

# Remove temp sys.path
#sys.path.remove( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
sys.path.remove( os.path.dirname(lCurFilePath) )                            