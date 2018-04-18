
# Internal action for reference manager
#
# Sergey <Neill3d> Solokhin 2018

from pyfbsdk import *

import inspect
import sys
import os

mSys = FBSystem()

lCurFilePath = inspect.currentframe().f_code.co_filename
sys.path.append( os.path.dirname(lCurFilePath) )
#sys.path.append( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )

import FbxShadersGraphImport as imp
import FbxShadersGraphMisc as misc

def PickRefName():
    lFileRefName = ''
    lBtnClicked, lFileRefName = FBMessageBoxGetUserValue( 'File Reference', 'Enter your file reference name', '', FBPopupInputType.kFBPopupString, 'OK', 'Cancel', None, 1, True )
    if lBtnClicked == 2: return lBtnClicked, lFileRefName

    lFileRefName = lFileRefName.replace(' ','')
    while lFileRefName is '':
        FBMessageBox('File Reference', 'Error, please enter a file reference name', "OK")
        lBtnClicked, lFileRefName = PickRefName()
        if lBtnClicked == 2: break
    
    return lBtnClicked, lFileRefName

def AddAReference():
    

    lDialog = FBFilePopup()
    lDialog.Style = FBFilePopupStyle.kFBFilePopupOpen
    lDialog.Filter = '*.fbx'
    
    if lDialog.Execute():
        fname = lDialog.FullFilename

        lBtnClicked, lName = PickRefName()
        if lBtnClicked == 2: return  
        
        lNameList = FBStringList()
        lNameList.Add(lName)    
    
        lStatus = mSys.Scene.NamespaceImportToMultiple( lNameList, fname, True )
        # import shaders graph
        if True == lStatus:
            
            # create a description holder
            descHolder = FBCreateObject(misc.gDescAssetPath, misc.gDescTitle, lName + '_DescHolder')
            descProp = None
            
            if descHolder is not None:
                descProp = descHolder.PropertyList.Find('Reference Model')
                    
            
            # we do shaders graph copies
            for name in lNameList:
                objNS = mSys.Scene.NamespaceGet( name  )
        
                if descProp is not None:
                    FBConnect(objNS, descProp)
                
                if objNS.IsLoaded:
                    modelsMask = []
                    imp.DoFbxShadersGraphImport(True, 1, objNS, modelsMask)
                    if descHolder is not None:
                        prop = descHolder.PropertyList.Find('Store')
                        if prop is not None:
                            prop()
                else:
                    FBMessageBox("Reference A File", "Failed to load a reference", "Ok")


# main

AddAReference()

# Remove temp sys.path
#sys.path.remove( os.path.join( os.path.dirname(lCurFilePath), 'MBFileRefAdvanced' ) )
sys.path.remove( os.path.dirname(lCurFilePath) )