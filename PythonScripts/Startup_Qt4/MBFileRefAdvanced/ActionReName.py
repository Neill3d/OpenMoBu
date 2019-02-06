
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

#import FbxShadersGraphImport as imp
import FbxShadersGraphMisc as misc
lRefName = ''
lArg1 = ''
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
    
    # change name of shaders namespace and change name of description holder
    lOldName = lArg1
    lNewName = lNSObj.LongName
    
    if True == lScene.NamespaceExist( str(lOldName) + 'S' ):
        lScene.NamespaceRename( str(lOldName) + 'S', str(lNewName) + 'S' )
    
    # holder
    misc.DescriptionRename(lNSObj, str(lNewName))    
    
    
# Remove temp sys.path
#sys.path.remove( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
sys.path.remove( os.path.dirname(lCurFilePath) )                            