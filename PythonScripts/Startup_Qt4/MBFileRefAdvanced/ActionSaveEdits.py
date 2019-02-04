
# Internal action for reference manager
#
# Sergey <Neill3d> Solokhin 2018

from pyfbsdk import *
import os

lRefName = ''
lSystem = FBSystem()
lScene = lSystem.Scene

if len(lRefName) > 0:
    
    for ns in lScene.Namespaces:
        if isinstance(ns, FBFileReference) and lRefName == ns.LongName:
            if ns.GetContentModified( FBPlugModificationFlag.kFBContentAllModifiedMask ):
                
                text = ns.GetRefEdit()
                
                lDialog = FBFilePopup()
                lDialog.Filter = '*.py'
                lDialog.Style = FBFilePopupStyle.kFBFilePopupSave
                
                if (lDialog.Execute()):
                    fname = lDialog.FullFilename
                    
                    with open(fname, "w") as text_file:
                        text_file.write(text)            