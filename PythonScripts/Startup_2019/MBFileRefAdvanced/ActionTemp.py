
from pyfbsdk import *
import os

lRefName = 'test'
lSystem = FBSystem()
lScene = lSystem.Scene


if len(lRefName) > 0:
    
    for ns in lScene.Namespaces:
        if isinstance(ns, FBFileReference) and lRefName == ns.LongName:
            if False == os.path.isfile(ns.ReferenceFilePath):
                FBMessageBox('Reference A File', 'Referenced file is not found!', 'Ok')
