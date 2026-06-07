
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
lArg1 = ''

try:

    import FbxShadersGraphImport as imp
    import FbxShadersGraphMisc as misc
    lSystem = FBSystem()
    lScene = lSystem.Scene

    lNSObj = None

    if len(lRefName) > 0:
        
        for ns in lScene.Namespaces:
            if isinstance(ns, FBFileReference) and lRefName == ns.LongName:
                if not os.path.isfile(ns.ReferenceFilePath):
                    FBMessageBox('Reference A File', 'Referenced file is not found!', 'Ok')
                else:            
                    lNSObj = ns
                break

        filename = lArg1
        if lNSObj is not None:
            filename = lNSObj.ReferenceFilePath
        
        # check if we have any shaders graph connected to delete
        anyInstances = False
        for namespace in lScene.Namespaces:
            if namespace != lNSObj:
                if isinstance(namespace, FBFileReference):
                    if namespace.ReferenceFilePath == filename:
                        anyInstances = True
        
        if not anyInstances:
            misc.DeleteShadersGraph(lNSObj, filename)
            # let's find any Description Holder connection
            misc.DescriptionDelete(lNSObj)    
            
            # remove from monitor manager
            lMon = FBFileMonitoringManager()
            lMon.RemoveFileFromMonitor(filename)
            
finally:
    # Remove temp sys.path
    sys.path.remove( os.path.dirname(lCurFilePath) )                            