
# Internal action for reference manager
#
# Sergey <Neill3d> Solokhin 2018-2026

from pyfbsdk import *

import inspect
import sys
import os

lCurFilePath = inspect.currentframe().f_code.co_filename

path = os.path.dirname(lCurFilePath)
sys.path.append(path)

# MoBu substitutes these at the top level before executing the script
lRefName = ''

try:

    import FbxShadersGraphImport as imp
    import FbxShadersGraphMisc as misc

    lSystem = FBSystem()
    lScene = lSystem.Scene

    objNS = None

    if len(lRefName) > 0:
        
        for ns in lScene.Namespaces:
            if isinstance(ns, FBFileReference) and lRefName == ns.LongName:
                
                objNS = ns
                break

    if objNS is not None:
        
        pNewPath = ""
        
        lDialog = FBFilePopup()
        lDialog.Style = FBFilePopupStyle.kFBFilePopupOpen
        lDialog.Filter = '*.fbx'
        
        if lDialog.Execute():
            pNewPath = lDialog.FullFilename    
        
        pOldPath = objNS.ReferenceFilePath
        applyEdits = False
        
        lOption = FBMessageBox('Change A Path', 'Is it a new reference file or a new path for the old one ?', 'New Ref', 'New Path', 'Cancel')

        applyEdits = (lOption == 2)
        
        if lOption < 3 and len(pNewPath) > 0:
            
            if True == objNS.SwapReferenceFilePath( str(pNewPath), applyEdits, applyEdits ):
            
                if False == applyEdits:
                
                    # do we have any other instances with old path
                    anyOtherInstance = False
                    for ns in lScene.Namespaces:
                        if isinstance(ns, FBFileReference):
                            refPath = ns.ReferenceFilePath
                            if pOldPath == refPath and ns != objNS:
                                anyOtherInstance = True
                                break
                    
                    if True == anyOtherInstance:
                        misc.DisconnectAllShaders(objNS)
                        misc.DescriptionDisconnect(objNS)
                    else:
                        misc.DeleteShadersGraph(objNS, pOldPath)
                    
                    modelsMask = []
                    objNS.RevertRefEdit()
                    imp.DoFbxShadersGraphImport(True, 2, objNS, modelsMask)
                    
                    # holder
                    base = os.path.splitext(pNewPath)[0]
                    newXml = base + '.xml'
                    misc.DescriptionStore(objNS, str(newXml))
                    
                else:
                    misc.SwapShadersGraph(objNS, str(pOldPath), str(pNewPath))         
                    
finally:
    # Remove temp sys.path
    sys.path.remove( os.path.dirname(lCurFilePath) )