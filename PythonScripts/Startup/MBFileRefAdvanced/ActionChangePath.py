
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
                for ns in self.mSys.Scene.Namespaces:
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
                
            
# Remove temp sys.path
#sys.path.remove( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
sys.path.remove( os.path.dirname(lCurFilePath) )