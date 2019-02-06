
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
#import FbxShadersGraphMisc as misc

lRefName = ''
lSystem = FBSystem()
lScene = lSystem.Scene

if len(lRefName) > 0:
    
    for ns in lScene.Namespaces:
        if isinstance(ns, FBFileReference) and lRefName == ns.LongName:
            
            ns.IsLoaded = False
            ns.IsLoaded = True
            
            ns.RevertRefEdit()
            # if we have shaders graph, let's assign it from scratch
            imp.RestoreShadersGraph(ns)            
            
            
# Remove temp sys.path
#sys.path.remove( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
sys.path.remove( os.path.dirname(lCurFilePath) )