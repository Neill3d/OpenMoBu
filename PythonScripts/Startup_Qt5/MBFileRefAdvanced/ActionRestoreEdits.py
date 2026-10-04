
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
                
finally:
    # Remove temp sys.path
    sys.path.remove( os.path.dirname(lCurFilePath) )